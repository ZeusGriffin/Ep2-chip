param(
    [string]$Port = ""
)

$ErrorActionPreference = "Stop"

function Write-Step($text) {
    Write-Host ""
    Write-Host "==> $text" -ForegroundColor Cyan
}

function Fail($message) {
    Write-Host ""
    Write-Host "ERROR: $message" -ForegroundColor Red
    exit 1
}

$ProjectRoot = Split-Path -Parent $PSScriptRoot
$FirmwareDir = Join-Path $ProjectRoot "firmware"

if (-not (Test-Path (Join-Path $FirmwareDir "platformio.ini"))) {
    Fail "Could not find firmware\platformio.ini. Keep this script inside the project folder."
}

Write-Host "============================================================" -ForegroundColor White
Write-Host " CARDPUTER ADV PHONE MIRROR - WINDOWS FLASHER" -ForegroundColor White
Write-Host "============================================================" -ForegroundColor White
Write-Host "Project: $ProjectRoot"

# Find a usable Python 3 command.
$PythonExe = $null
$PythonPrefix = @()

if (Get-Command py.exe -ErrorAction SilentlyContinue) {
    try {
        & py.exe -3 --version | Out-Null
        $PythonExe = "py.exe"
        $PythonPrefix = @("-3")
    } catch {}
}

if (-not $PythonExe -and (Get-Command python.exe -ErrorAction SilentlyContinue)) {
    try {
        & python.exe --version | Out-Null
        $PythonExe = "python.exe"
    } catch {}
}

if (-not $PythonExe) {
    Fail "Python 3 was not found. Install Python 3 from python.org, check 'Add Python to PATH', then run FLASH_CARDPUTER_WINDOWS.bat again."
}

function Run-Python {
    param([Parameter(ValueFromRemainingArguments = $true)][string[]]$Args)
    & $PythonExe @PythonPrefix @Args
    if ($LASTEXITCODE -ne 0) {
        throw "Python command failed with exit code $LASTEXITCODE"
    }
}

Write-Step "Checking PlatformIO"
$PioReady = $true
try {
    Run-Python -m platformio --version
} catch {
    $PioReady = $false
}

if (-not $PioReady) {
    Write-Host "PlatformIO is not installed yet. Installing it for this Windows account..." -ForegroundColor Yellow
    try {
        Run-Python -m pip install --user --upgrade platformio
    } catch {
        Fail "PlatformIO installation failed. Check your internet connection and Python installation."
    }
}

Write-Step "Put the Cardputer ADV into download mode"
Write-Host "1. Set the Cardputer ADV side power switch to OFF."
Write-Host "2. Hold the G0 button."
Write-Host "3. While holding G0, connect the Cardputer ADV to this PC with a USB-C DATA cable."
Write-Host "4. Release G0 after the USB cable is connected."
Write-Host ""
Write-Host "This is the ESP32-S3 download/bootloader mode used for firmware flashing." -ForegroundColor DarkGray
Write-Host ""
Read-Host "Press ENTER when the Cardputer ADV is connected in download mode"

Write-Step "Showing detected serial devices"
try {
    Run-Python -m platformio device list
} catch {
    Write-Host "Could not list serial ports, but the flash step can still try auto-detection." -ForegroundColor Yellow
}

Write-Step "Building Cardputer Mirror firmware"
try {
    Run-Python -m platformio run -d $FirmwareDir
} catch {
    Fail "Firmware build failed. Scroll up for the compiler error."
}

Write-Step "Flashing firmware to Cardputer ADV"
try {
    if ([string]::IsNullOrWhiteSpace($Port)) {
        Write-Host "Using PlatformIO automatic COM-port detection."
        Run-Python -m platformio run -d $FirmwareDir -t upload
    } else {
        Write-Host "Using requested port: $Port"
        Run-Python -m platformio run -d $FirmwareDir -t upload --upload-port $Port
    }
} catch {
    Write-Host ""
    Write-Host "FLASH FAILED" -ForegroundColor Red
    Write-Host "Try these checks:" -ForegroundColor Yellow
    Write-Host "- Make sure the USB-C cable supports DATA, not charging only."
    Write-Host "- Put the ADV back into download mode: switch OFF, hold G0, connect USB, release G0."
    Write-Host "- Close M5Burner, Arduino Serial Monitor, or any app using the COM port."
    Write-Host "- If auto-detection chose the wrong port, run from Command Prompt:"
    Write-Host "  FLASH_CARDPUTER_WINDOWS.bat -Port COM5"
    Write-Host "  Replace COM5 with the port shown for the Cardputer ADV."
    exit 1
}

Write-Host ""
Write-Host "============================================================" -ForegroundColor Green
Write-Host " FLASH COMPLETE" -ForegroundColor Green
Write-Host "============================================================" -ForegroundColor Green
Write-Host ""
Write-Host "Next:"
Write-Host "1. Disconnect the USB cable."
Write-Host "2. Set the Cardputer ADV power switch to ON."
Write-Host "3. Power/reconnect it normally."
Write-Host "4. The display should show CARDPUTER MIRROR."
Write-Host "5. Look for Wi-Fi network: CardputerMirror"
Write-Host "   Password: cardputer"
Write-Host "   Receiver: 192.168.4.1:9000"
Write-Host ""
Write-Host "If the normal firmware does not start, factory firmware can be restored with M5Burner." -ForegroundColor DarkGray
exit 0
