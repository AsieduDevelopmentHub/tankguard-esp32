#pragma once
#include <stdint.h>
namespace config {
constexpr int TRIG = 4, ECHO = 3, RELAY = 5;
// Hardware output deliberately unavailable in this starter.
constexpr bool OUTPUT_ENABLED = false;
// Sensor-face distances, not external tank dimensions. Set after measuring.
constexpr bool CALIBRATED = false;
constexpr float EMPTY_CM = 100.0f, FULL_CM = 15.0f;
constexpr float MIN_SPAN_CM = 10.0f;
// Mixed short/long echoes are multipath, not surface ripple.
constexpr float MAX_ECHO_SPREAD_CM = 10.0f;
// Larger than sensor noise or a real 1 s fill step; smaller than a false echo jump.
constexpr float MAX_LEVEL_STEP = 15.0f;
constexpr uint32_t SAMPLE_MS = 1000;
}
