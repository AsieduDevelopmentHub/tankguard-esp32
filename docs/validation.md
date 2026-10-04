# Validation — 2026-10-04

- Host C++11 controller and echo-policy suite: 61 checks passed with -Wall -Wextra -Werror.
- The same suite passed with -DNDEBUG, so the checks are not compiled out.
- PlatformIO bench environment: build succeeded.
- PlatformIO sensor environment: build succeeded.
- ESP32-C3 profile, espressif32 6.9.0, Arduino ESP32 2.0.17.
- No physical board, sensor, relay, pump or 30 m link tested.
- Physical relay output remains unconditionally LOW in both environments.
