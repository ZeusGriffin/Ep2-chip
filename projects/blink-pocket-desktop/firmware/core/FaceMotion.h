#pragma once

#include <cstdint>

enum class BlinkFaceState : uint8_t {
  Idle,
  Listening,
  Thinking,
  Speaking,
  Reacting
};

struct FaceFrame {
  float gazeX = 0.0f;      // -1 left .. +1 right
  float gazeY = 0.0f;      // -1 up   .. +1 down
  float eyeOpen = 1.0f;    // 0 closed .. 1 open
  float mouthOpen = 0.0f;  // 0 closed .. 1 open
  float bob = 0.0f;        // subtle vertical personality motion
  BlinkFaceState state = BlinkFaceState::Idle;
};

class FaceMotion {
 public:
  explicit FaceMotion(uint32_t seed = 0x5A17u);

  void reset(uint32_t nowMs);
  void setState(BlinkFaceState state, uint32_t nowMs);
  void react(uint32_t nowMs);
  void setSpeechLevel(float level01);
  FaceFrame update(uint32_t nowMs);

 private:
  uint32_t rng_;
  uint32_t lastUpdateMs_ = 0;
  uint32_t nextBlinkMs_ = 0;
  uint32_t blinkStartMs_ = 0;
  uint32_t blinkDurationMs_ = 130;
  uint32_t nextGazeMs_ = 0;
  uint32_t reactUntilMs_ = 0;
  BlinkFaceState state_ = BlinkFaceState::Idle;
  float speechLevel_ = 0.0f;
  float gazeX_ = 0.0f;
  float gazeY_ = 0.0f;
  float targetGazeX_ = 0.0f;
  float targetGazeY_ = 0.0f;
  bool blinking_ = false;

  uint32_t rand32();
  float randRange(float lo, float hi);
  uint32_t randRangeMs(uint32_t lo, uint32_t hi);
  static float clamp01(float v);
  static float approach(float current, float target, float maxDelta);
};
