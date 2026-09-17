#include "FaceMotion.h"

#include <cmath>

FaceMotion::FaceMotion(uint32_t seed) : rng_(seed ? seed : 0x5A17u) {}

void FaceMotion::reset(uint32_t nowMs) {
  lastUpdateMs_ = nowMs;
  nextBlinkMs_ = nowMs + randRangeMs(1800, 4200);
  nextGazeMs_ = nowMs + randRangeMs(500, 1600);
  blinkStartMs_ = 0;
  reactUntilMs_ = 0;
  blinking_ = false;
  gazeX_ = gazeY_ = targetGazeX_ = targetGazeY_ = 0.0f;
  speechLevel_ = 0.0f;
  state_ = BlinkFaceState::Idle;
}

void FaceMotion::setState(BlinkFaceState state, uint32_t nowMs) {
  state_ = state;
  if (state == BlinkFaceState::Listening) {
    targetGazeX_ = 0.0f;
    targetGazeY_ = -0.08f;
    nextGazeMs_ = nowMs + 1200;
  } else if (state == BlinkFaceState::Thinking) {
    nextGazeMs_ = nowMs;
  } else if (state == BlinkFaceState::Speaking) {
    targetGazeX_ = 0.0f;
    targetGazeY_ = 0.0f;
  }
}

void FaceMotion::react(uint32_t nowMs) {
  state_ = BlinkFaceState::Reacting;
  reactUntilMs_ = nowMs + 650;
  targetGazeX_ = randRange(-0.45f, 0.45f);
  targetGazeY_ = -0.30f;
}

void FaceMotion::setSpeechLevel(float level01) {
  speechLevel_ = clamp01(level01);
}

FaceFrame FaceMotion::update(uint32_t nowMs) {
  uint32_t dtMs = nowMs - lastUpdateMs_;
  if (dtMs > 100) dtMs = 100;
  lastUpdateMs_ = nowMs;

  if (state_ == BlinkFaceState::Reacting && nowMs >= reactUntilMs_) {
    state_ = BlinkFaceState::Idle;
    nextGazeMs_ = nowMs + randRangeMs(400, 1000);
  }

  if (!blinking_ && nowMs >= nextBlinkMs_) {
    blinking_ = true;
    blinkStartMs_ = nowMs;
    blinkDurationMs_ = randRangeMs(105, 165);
  }

  float eyeOpen = 1.0f;
  if (blinking_) {
    const uint32_t elapsed = nowMs - blinkStartMs_;
    if (elapsed >= blinkDurationMs_) {
      blinking_ = false;
      eyeOpen = 1.0f;
      nextBlinkMs_ = nowMs + randRangeMs(1800, 4800);
    } else {
      const float p = static_cast<float>(elapsed) / static_cast<float>(blinkDurationMs_);
      eyeOpen = (p < 0.5f) ? (1.0f - p * 2.0f) : ((p - 0.5f) * 2.0f);
    }
  }

  if (nowMs >= nextGazeMs_) {
    switch (state_) {
      case BlinkFaceState::Idle:
        targetGazeX_ = randRange(-0.42f, 0.42f);
        targetGazeY_ = randRange(-0.22f, 0.18f);
        nextGazeMs_ = nowMs + randRangeMs(700, 2200);
        break;
      case BlinkFaceState::Listening:
        targetGazeX_ = randRange(-0.08f, 0.08f);
        targetGazeY_ = -0.08f;
        nextGazeMs_ = nowMs + randRangeMs(1000, 2200);
        break;
      case BlinkFaceState::Thinking:
        targetGazeX_ = randRange(-0.55f, 0.55f);
        targetGazeY_ = randRange(-0.34f, 0.12f);
        nextGazeMs_ = nowMs + randRangeMs(350, 850);
        break;
      case BlinkFaceState::Speaking:
        targetGazeX_ = randRange(-0.12f, 0.12f);
        targetGazeY_ = randRange(-0.08f, 0.08f);
        nextGazeMs_ = nowMs + randRangeMs(800, 1800);
        break;
      case BlinkFaceState::Reacting:
        nextGazeMs_ = nowMs + 500;
        break;
    }
  }

  const float dt = static_cast<float>(dtMs) / 1000.0f;
  const float gazeSpeed = (state_ == BlinkFaceState::Thinking) ? 1.6f : 0.75f;
  gazeX_ = approach(gazeX_, targetGazeX_, gazeSpeed * dt);
  gazeY_ = approach(gazeY_, targetGazeY_, gazeSpeed * dt);

  FaceFrame frame;
  frame.gazeX = gazeX_;
  frame.gazeY = gazeY_;
  frame.eyeOpen = clamp01(eyeOpen);
  frame.state = state_;

  const float t = static_cast<float>(nowMs % 4000u) / 4000.0f;
  frame.bob = 0.018f * std::sin(t * 6.2831853f);

  switch (state_) {
    case BlinkFaceState::Idle:
      frame.mouthOpen = 0.02f;
      break;
    case BlinkFaceState::Listening:
      frame.mouthOpen = 0.0f;
      break;
    case BlinkFaceState::Thinking:
      frame.mouthOpen = 0.04f;
      break;
    case BlinkFaceState::Speaking:
      frame.mouthOpen = 0.10f + 0.90f * speechLevel_;
      break;
    case BlinkFaceState::Reacting:
      frame.mouthOpen = 0.28f;
      break;
  }

  frame.mouthOpen = clamp01(frame.mouthOpen);
  return frame;
}

uint32_t FaceMotion::rand32() {
  uint32_t x = rng_;
  x ^= x << 13;
  x ^= x >> 17;
  x ^= x << 5;
  rng_ = x;
  return x;
}

float FaceMotion::randRange(float lo, float hi) {
  const float u = static_cast<float>(rand32() & 0x00FFFFFFu) / static_cast<float>(0x01000000u);
  return lo + (hi - lo) * u;
}

uint32_t FaceMotion::randRangeMs(uint32_t lo, uint32_t hi) {
  if (hi <= lo) return lo;
  return lo + (rand32() % (hi - lo + 1u));
}

float FaceMotion::clamp01(float v) {
  if (v < 0.0f) return 0.0f;
  if (v > 1.0f) return 1.0f;
  return v;
}

float FaceMotion::approach(float current, float target, float maxDelta) {
  if (current < target) {
    current += maxDelta;
    if (current > target) current = target;
  } else if (current > target) {
    current -= maxDelta;
    if (current < target) current = target;
  }
  return current;
}
