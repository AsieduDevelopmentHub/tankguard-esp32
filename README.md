# TankGuard

Standalone water-tank refill controller using ESP32-C3, HC-SR04 and PlatformIO, with fail-safe control logic and optional future IoT monitoring.

**Status: bench prototype. Both firmware environments lock the physical relay output OFF. This is not deployment-ready pump firmware.**

## Installation concept
A white IBC tank in a steel cage sits on the first-floor veranda of an unfinished building. The existing switched pump socket and controller are outside the adjacent boys’ quarters, beside ECG meters. The proposed cable path is about 30 m. One ESP32-C3 is planned near the tank; the long-distance control and power interfaces are not implemented or electrically specified.

## What works in this starter
- Internet-independent controller; no cloud libraries or credentials.
- Simulated level input for bench development, or HC-SR04 acquisition on short wires.
- Low-level confirmation, start/stop hysteresis, minimum off time.
- Immediate stop demand on an invalid measurement; stale-data timeout.
- Latched maximum-fill and no-rise faults; separate acknowledgment and arming.
- Always disarmed on boot. Hardware GPIO remains LOW regardless of demand.
- Portable C++ controller and echo-policy tests. GitHub Actions runs those tests and both PlatformIO environments.

## Quick start (PlatformIO in VS Code / Cursor)
Open this folder as a PlatformIO project. The generic ESP32-C3-DevKitM-1 board profile is used for the SuperMini; confirm flash size and USB behavior for your clone.

```sh
pio run -e bench
pio run -e bench -t upload
pio device monitor -b 115200
```
On Windows add `--upload-port COMx` / `--port COMx` when needed. If upload does not connect, hold BOOT, tap RESET, release BOOT, and retry. Use USB power only during first tests; avoid simultaneous external 5 V and USB unless your board supports it.

With serial monitor set to newline, enter `level 20`, wait for a sample, then `arm`. Demand starts after five seconds continuously low AND sixty seconds since boot/last stop. `level 90` stops demand. `invalid` latches a fault. Restore `level 50`, wait for a sample, then `ack` and `arm` separately. `stop` disarms. Demand is a software request, **not measured pump operation**.

## Sensor mode
Set real sensor-face distances and `CALIBRATED=true` in `include/config.h`, then build/upload `sensor`. This still cannot energize the output. Until calibrated, or if the empty-to-full span is under `MIN_SPAN_CM`, samples latch `fault=config`. A dead pulse, a batch whose echoes disagree by more than `MAX_ECHO_SPREAD_CM`, or a jump larger than `MAX_LEVEL_STEP` from the last accepted reading latches `fault=sensor`. Defaults are placeholders, not IBC measurements.

| Signal | Proposed connection |
|---|---|
| HC-SR04 VCC | Local regulated 5 V |
| HC-SR04 GND | ESP32/common GND |
| TRIG | GPIO4 |
| ECHO | 1 kΩ to junction at GPIO3; junction through two 1 kΩ in series to GND |
| Driver request | GPIO5, locked LOW in starter |

GPIO numbers are chip GPIO identifiers. Keep sensor wires short. Do not run raw TRIG/ECHO/GPIO over the 30 m Cat6. Level percentage is calibrated water height, not certified tank volume.

The echo divider above is 5 V through 1 kΩ into 2 kΩ, about 3.33 V, with no margin for resistor and supply tolerance. Before trusting GPIO3, use a divider aimed near 3.0 V, such as 2.2 kΩ in series and 3.3 kΩ to ground. Confirm the SuperMini actually breaks out GPIO3, GPIO4, and GPIO5; the build profile is the generic DevKitM-1.

## Hardware still to resolve
Confirmed: 5 V/2 A adapter, ESP32-C3 SuperMini, HC-SR04, three 1 kΩ resistors, BC547, bare 3 V Songle relay, measured coil 35 Ω, Cat6 and existing pump socket.

The relay coil would draw about 86 mA at 3 V or 94 mA at 3.3 V before driver drop. This is too close to BC547's 100 mA maximum for a robust design. Verify coil identity/resistance, regulator headroom and choose a driver with margin. A flyback diode, driver resistor and external off-state pull-down are required. Never power the 3 V coil directly from 5 V. Relay contact motor rating and pump starting current remain unverified; 10 A resistive marking is insufficient. No mains wiring is specified here.

See [commissioning](docs/commissioning.md) before extending to live control. Optional ThingSpeak telemetry, GitHub Pages dashboard, float interlock, independent watchdog and 30 m interface are roadmap items, not implemented features.

## Test controller on a host
```sh
c++ -std=c++11 -Wall -Wextra -Werror -Iinclude tests/controller_test.cpp -o /tmp/tankguard-test
/tmp/tankguard-test
```
Use an equivalent temporary executable path on Windows with a C++ compiler installed.

## Structure
`src/main.cpp`: sensor acquisition, serial commands and locked output. `include/controller.h`: portable state machine. `include/level.h`: echo acceptance and percent conversion. `include/config.h`: pins and calibration. `tests/`: host checks. `docs/`: hardware decisions and commissioning.

## References
- [PlatformIO ESP32-C3 profile](https://docs.platformio.org/en/latest/boards/espressif32/esp32-c3-devkitm-1.html)
- [Espressif GPIO hardware guidance](https://docs.espressif.com/projects/esp-hardware-design-guidelines/en/latest/esp32c3/schematic-checklist.html)

No license has been selected yet.

## Maintainer

Maintained by [Asiedu Minta Kwaku](https://asiedudevhub.auralenx.com/), a software developer and IoT & embedded systems engineer based in Ghana.

For related work and project enquiries, visit my [portfolio](https://asiedudevhub.auralenx.com/) or connect on [LinkedIn](https://www.linkedin.com/in/asiedudevelopmenthub).
