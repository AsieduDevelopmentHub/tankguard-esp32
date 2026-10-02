#pragma once
#include <stdint.h>
namespace config {
constexpr int TRIG = 4, ECHO = 3, RELAY = 5;
// Hardware output deliberately unavailable in this starter.
constexpr bool OUTPUT_ENABLED = false;
// Sensor-face distances, not external tank dimensions. Set after measuring.
constexpr bool CALIBRATED = false;
constexpr float EMPTY_CM = 100.0f, FULL_CM = 15.0f;
constexpr uint32_t SAMPLE_MS = 1000;
}
