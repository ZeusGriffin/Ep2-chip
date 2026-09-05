@echo off
setlocal
title Cardputer ADV Phone Mirror - One Click Flasher
cd /d "%~dp0"

echo ============================================================
echo   CARDPUTER ADV PHONE MIRROR - ONE CLICK WINDOWS FLASHER
echo ============================================================
echo.

powershell.exe -NoProfile -ExecutionPolicy Bypass -File "%~dp0windows\flash_cardputer.ps1" %*
set EXITCODE=%ERRORLEVEL%

echo.
if not "%EXITCODE%"=="0" (
  echo Flash process ended with an error. See the message above.
) else (
  echo Flash process completed successfully.
)
echo.
pause
exit /b %EXITCODE%
