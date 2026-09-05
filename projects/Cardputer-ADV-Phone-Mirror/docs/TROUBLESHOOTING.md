# Troubleshooting

## Cardputer does not appear over USB

- Try a known data-capable USB-C cable.
- Try another USB port without a hub.
- Confirm the device can enter its normal flashing/download mode.
- Record the exact Windows Device Manager / macOS USB identity rather than guessing.

## Firmware builds but upload fails

- Record the exact PlatformIO upload error.
- Verify the selected serial port.
- Lower upload speed if the connection is unstable.
- Do not randomly change flash-size/partition settings without checking the actual ADV hardware.

## Waiting screen appears but PC cannot connect

- Confirm SSID `CardputerMirror` is visible.
- Confirm password `cardputer`.
- Confirm sender host is `192.168.4.1` and port `9000`.
- Check serial log for sender connection attempts.

## "Bad stream header"

Sender and receiver disagree about the wire protocol. Confirm the sender writes:

1. ASCII `CMIR`
2. 4-byte unsigned JPEG length in big-endian order
3. JPEG bytes

## "Frame too large"

- Reduce JPEG quality.
- Ensure the sender resizes to 240x135 before encoding.
- Inspect actual JPEG byte size.
- Do not simply raise the receiver limit until free-heap behavior is known.

## "JPEG decode failed"

- Save one failing JPEG from the sender and verify it opens normally on a computer.
- Confirm baseline JPEG compatibility.
- Verify frame buffer content/length.
- Check for partial TCP reads or incorrect length framing.

## Low FPS / high latency

Tune in this order:

1. lower JPEG quality
2. reduce FPS target if queue pressure exists
3. keep native 240x135 output
4. measure network send time
5. measure decode/draw time

Do not add complicated codecs before measuring the current bottleneck.

## iPhone broadcast starts but Cardputer remains blank

- Confirm the iPhone is on `CardputerMirror` Wi-Fi.
- Confirm the extension has local-network permission if prompted.
- Confirm TCP connection in Cardputer serial log.
- Check whether normal home/app UI works before testing secure/DRM media.

## DRM / protected content is black

This can be expected behavior under ReplayKit. Treat it as an OS/content-protection limitation unless ordinary UI is also black.
