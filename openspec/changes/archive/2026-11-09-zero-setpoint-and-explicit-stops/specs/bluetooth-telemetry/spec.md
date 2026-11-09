## MODIFIED Requirements

### Requirement: Arduino SHALL send a periodic telemetry frame

The system SHALL send a telemetry frame over Hardware Serial (D0/D1, 115200 baud) once every 200 ms, driven by a timer in the main loop independently of command arrival. The frame SHALL have the form `ack <n> <linear> <angular> mL=<ml> mR=<mr> oL=<ol> oR=<or> iL=<il> iR=<ir>`, where `<n>` is a running counter of processed valid control commands (`set_speed`, `stop`, `power_stop`), `<linear>`/`<angular>` are the most recently commanded speeds (mm/s and rad/s, both `0` before any command has arrived, and reset to `0` after a `stop` or `power_stop` command), and the `mL/mR/oL/oR/iL/iR` fields carry the current measured speed, PID output and PID integral of each wheel. The frame SHALL be sent regardless of drive state. Parsing or applying a command SHALL NOT itself emit a frame. The periodic frame keeps the RFCOMM link continuously bidirectional; this keepalive is required because an HC-06 link with one-directional traffic drops at ~5.2 s, while continuous bidirectional traffic is verified to survive ≥60 s on production hardware.

#### Scenario: Telemetry is sent on the timer

- **WHEN** 200 ms have elapsed since the previous telemetry frame
- **THEN** a frame is sent regardless of whether a new command has arrived

#### Scenario: Frame echoes the last commanded speed

- **WHEN** the last valid command received was `set_speed 30.0 0.0`
- **THEN** subsequent telemetry frames report the fields `ack <n> 30.0 0.00` with `<n>` equal to the running command counter

#### Scenario: Stop command zeroes the echoed speed

- **WHEN** `stop` or `power_stop` arrives after `set_speed 30.0 0.0`
- **THEN** subsequent telemetry frames report `ack <n> 0.0 0.00`, and `<n>` includes the processed stop command

#### Scenario: A command does not trigger an extra frame

- **WHEN** a valid control command arrives between two timer ticks
- **THEN** the command is applied and the counter increments, but no frame is emitted from the command handler

#### Scenario: Invalid line does not change telemetry

- **WHEN** a line that does not match `set_speed`, `stop`, or `power_stop` arrives
- **THEN** neither the command counter nor the echoed last commanded speed changes, and frames continue to be sent only by the periodic timer
