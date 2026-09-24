#include "CompanionInputs.h"
#include <cmath>

CompanionInputs::CompanionInputs(MotionThresholds thresholds) : t_(thresholds) {}

void CompanionInputs::reset(uint32_t nowMs) {
  last_ = MotionSample{};
  haveLast_ = false;
  lastMotionMs_ = nowMs;
  cooldownUntilMs_ = 0;
  stillReported_ = false;
}

CompanionEvent CompanionInputs::updateMotion(const MotionSample& s, uint32_t nowMs) {
  const float accelMag = mag3(s.ax, s.ay, s.az);
  const float gyroMag = mag3(s.gx, s.gy, s.gz);

  if (!haveLast_) {
    last_ = s;
    haveLast_ = true;
    lastMotionMs_ = nowMs;
    return CompanionEvent::None;
  }

  const float lastAccelMag = mag3(last_.ax, last_.ay, last_.az);
  const float accelDelta = absf(accelMag - lastAccelMag);
  const float axisDelta =
      absf(s.ax - last_.ax) + absf(s.ay - last_.ay) + absf(s.az - last_.az);
  last_ = s;

  const bool moved = accelDelta >= t_.moveDeltaG || axisDelta >= (t_.moveDeltaG * 1.8f);
  if (moved) {
    lastMotionMs_ = nowMs;
    stillReported_ = false;
  }

  if (nowMs < cooldownUntilMs_) return CompanionEvent::None;

  if (gyroMag >= t_.shakeGyro) {
    cooldownUntilMs_ = nowMs + t_.cooldownMs;
    return CompanionEvent::Shake;
  }

  if (absf(accelMag - 1.0f) >= t_.pickupDeltaG) {
    cooldownUntilMs_ = nowMs + t_.cooldownMs;
    return CompanionEvent::PickedUp;
  }

  if (moved) {
    cooldownUntilMs_ = nowMs + t_.cooldownMs;
    return CompanionEvent::Motion;
  }

  if (!stillReported_ && (nowMs - lastMotionMs_) >= t_.stillMs) {
    stillReported_ = true;
    return CompanionEvent::StillLong;
  }

  return CompanionEvent::None;
}

CompanionEvent CompanionInputs::touchNext() { return CompanionEvent::TouchNext; }
CompanionEvent CompanionInputs::touchSelect() { return CompanionEvent::TouchSelect; }

float CompanionInputs::absf(float v) { return v < 0.0f ? -v : v; }

float CompanionInputs::mag3(float x, float y, float z) {
  return std::sqrt(x * x + y * y + z * z);
}
