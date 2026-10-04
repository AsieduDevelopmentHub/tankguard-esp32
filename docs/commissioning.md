# Commissioning and scope

This starter deliberately cannot switch a pump. OUTPUT_ENABLED is false, enforced with static_assert, and the runtime writes LOW unconditionally. Removing this is an engineering change, not a commissioning command.

1. Verify actual board and relay pinouts, adapter voltage, GPIO reset behavior and output driver off-state pull-down.
2. Bench-test sensor at measured distances. HC-SR04 is not waterproof; choose suitable long-term humidity protection or a replacement sensor. Firmware rejects a batch whose echoes disagree by more than MAX_ECHO_SPREAD_CM, and a level jump larger than MAX_LEVEL_STEP from the last accepted sample.
3. Measure sensor-to-empty and sensor-to-full distances inside the IBC. Avoid cage/neck/wall echoes and fill stream. Confirm overflow margin and response delay. Do not infer capacity from the outside of the tank. The empty-to-full span must be at least MIN_SPAN_CM.
4. Review relay coil tolerance, supply regulator load and temperature, transistor current margin and flyback polarity. Do not rely on the ESP32 3V3 pin without regulator validation.
5. Design the 30 m link and power distribution. Current starter assumes LOCAL wiring; no long-wire transmitter/receiver is implemented. Cat6 is low voltage only. Cable loss, common-mode noise, protection and fail-off behavior need assessment.
6. Confirm pump motor rating, inrush and switching device capability, existing protection, weatherproof enclosure and separation from ECG meter equipment. The project does not authorize changes to utility meters.
7. Add an independent high-level float stop path and an independent means to stop the pump if firmware or switching hardware fails. Test loss of sensor, cable, power and controller reset.
8. Set thresholds and timing from real fill tests. Defaults: 30% start, 85% stop, 5 s confirmation, 60 s minimum off, 20 min maximum fill, 2 percentage points rise in 3 min. These are placeholders.
9. Add hardware-output support only after design review. Current faults and runtime state are not persisted; reboot always disarms, requiring local rearming after fresh readings.

Remaining: watchdog strategy, persistent calibration, physical arming/reset controls, manual timeout, pump-running feedback, telemetry and dashboard. Network services must never gate the local stop logic.
