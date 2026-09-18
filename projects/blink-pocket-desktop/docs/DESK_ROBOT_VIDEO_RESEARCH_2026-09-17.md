# NODE / BLINK — Desk Robot Video Integration Research
Date: 2026-09-17
Reference video: https://youtu.be/4Z8oJTYTInc

## Confirmed reference features
- animated eyes / emotion states
- MPU6050 motion sensing
- live weather
- clock and world clock
- touch navigation
- compact ESP32 desk-companion UX

## GitHub references reviewed
- Misjre/QBIT-S3 and SeanChangX/QBIT: touch navigation, weather, clock, notification history, games, MQTT/Home Assistant, browser flasher
- Shri7ul/ESP32-MicroBot: moods, weather/forecast, NTP clock, reminders, notifications, setup portal, non-blocking/dual-core architecture
- merberg-ai/YETI: MPU6050/MPU6500 motion reactions, weather, clock, WebUI, preferences, sleep, diagnostics
- HamzaYslmn/esp-bridge-mcp-robot: ESP32 animated face controlled from AI/desktop tooling

## Added to NODE firmware architecture
1. Keep existing FaceMotion core.
2. Add hardware-neutral CompanionInputs motion/touch event engine.
3. Board adapter translates real MPU6050 readings into MotionSample.
4. Event map:
   - Motion -> short face reaction
   - Shake -> startled/excited reaction
   - PickedUp -> attentive reaction
   - StillLong -> sleepy/idle behavior
   - TouchNext -> cycle card/app
   - TouchSelect -> enter/select
5. Weather/clock stay services, not hard-wired into the face renderer.
6. Keep all animation and input handling non-blocking.

## Verification
CompanionInputs.cpp + unit test compiled and passed under C++17 before repository commit.

## Board-specific work intentionally NOT guessed
Do not assign MPU6050, touch, display, speaker, or SD GPIOs until the exact board and current pin map are verified.

## Licensing
Reference repositories use different licenses. Reuse concepts/API patterns only unless a source license explicitly permits copying. NODE code here is an original hardware-neutral implementation.
