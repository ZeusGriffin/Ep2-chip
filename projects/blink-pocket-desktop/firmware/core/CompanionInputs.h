#pragma once
#include <cstdint>

enum class CompanionEvent : uint8_t {
  None,
  TouchNext,
  TouchSelect,
  Motion,
  Shake,
  PickedUp,
  StillLong
};

struct MotionSample {
  float ax = 0.0f;
  float ay = 0.0f;
  float az = 1.0f;
  float gx = 0.0f;
  float gy = 0.0f;
  float gz = 0.0f;
};

struct MotionThresholds {
  float moveDeltaG = 0.18f;
  float shakeGyro = 120.0f;
  float pickupDeltaG = 0.42f;
  uint32_t stillMs = 45000;
  uint32_t cooldownMs = 900;
};

class CompanionInputs {
 public:
  explicit CompanionInputs(MotionThresholds thresholds = {});
  void reset(uint32_t nowMs);
  CompanionEvent updateMotion(const MotionSample& s, uint32_t nowMs);
  CompanionEvent touchNext();
  CompanionEvent touchSelect();

 private:
  MotionThresholds t_;
  MotionSample last_{};
  bool haveLast_ = false;
  uint32_t lastMotionMs_ = 0;
  uint32_t cooldownUntilMs_ = 0;
  bool stillReported_ = false;

  static float absf(float v);
  static float mag3(float x, float y, float z);
};
