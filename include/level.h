#pragma once
#include <cmath>

namespace tankguard {
struct LevelEval {
  bool ok;
  float percent;
};

inline bool calibrationSpanOk(float emptyCm, float fullCm, float minSpanCm) {
  return std::isfinite(emptyCm) && std::isfinite(fullCm) && std::isfinite(minSpanCm) &&
    minSpanCm > 0.f && fullCm >= 2.f && emptyCm > fullCm && (emptyCm - fullCm) >= minSpanCm;
}

inline bool echoInRange(float cm, float emptyCm) {
  return std::isfinite(cm) && cm >= 2.f && cm <= 400.f && cm <= emptyCm + 5.f;
}

// Host-testable HC-SR04 policy. Disagreeing echoes and abrupt jumps are rejected
// so a median cannot turn a mixed batch into a false low level.
inline LevelEval levelFromEchoes(float a, float b, float c, float emptyCm, float fullCm,
    float highPercent, float maxSpreadCm, bool hasPrevious, float previousPercent,
    float maxStepPercent, float minSpanCm) {
  LevelEval out{false, 0.f};
  if(!calibrationSpanOk(emptyCm, fullCm, minSpanCm)) return out;
  if(!(maxSpreadCm > 0.f) || !std::isfinite(maxSpreadCm)) return out;
  if(!(maxStepPercent > 0.f) || !std::isfinite(maxStepPercent)) return out;
  if(!std::isfinite(highPercent)) return out;
  float cm[3] = {a, b, c};
  for(int i = 0; i < 3; i++) if(!echoInRange(cm[i], emptyCm)) return out;
  for(int i = 0; i < 2; i++) for(int j = i + 1; j < 3; j++) if(cm[j] < cm[i]) {
    float v = cm[i]; cm[i] = cm[j]; cm[j] = v;
  }
  if(cm[2] - cm[0] > maxSpreadCm) return out;
  float span = emptyCm - fullCm;
  float nearest = 100.f * (emptyCm - cm[0]) / span;
  float distance = nearest >= highPercent ? cm[0] : cm[1];
  float pct = 100.f * (emptyCm - distance) / span;
  if(pct < 0.f) pct = 0.f;
  if(pct > 100.f) pct = 100.f;
  if(hasPrevious && std::isfinite(previousPercent) && std::fabs(pct - previousPercent) > maxStepPercent)
    return out;
  out.ok = true;
  out.percent = pct;
  return out;
}
}
