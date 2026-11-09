## MODIFIED Requirements

### Requirement: Parse set_speed commands over Hardware Serial

The firmware SHALL read incoming lines from the Bluetooth channel via Hardware Serial (D0/D1, 115200 baud) and parse commands in the text form `set_speed <linear_speed> <angular_speed>`, where `linear_speed` is in mm/s and `angular_speed` in rad/s, both possibly negative floats. A valid command SHALL be applied through the existing `set_speed()` motion controller, including the zero setpoint `set_speed 0.0 0.0`, which SHALL NOT be rewritten into a stop action. Lines that do not match the syntax, that contain non-numeric values, or that contain extra tokens SHALL be ignored without affecting the current command.

#### Scenario: Valid command is applied

- **WHEN** the line `set_speed 90 0.65` arrives over Hardware Serial
- **THEN** the motion controller receives `set_speed(90, 0.65)` and the wheels respond accordingly

#### Scenario: Reverse command is applied

- **WHEN** the line `set_speed -60 0.0` arrives
- **THEN** the robot drives in reverse at 60 mm/s

#### Scenario: Command with turn is applied

- **WHEN** the line `set_speed 120 1.2` arrives
- **THEN** the robot drives forward while turning, per the differential drive model

#### Scenario: Zero setpoint is applied as a setpoint

- **WHEN** the line `set_speed 0.0 0.0` arrives while the robot is moving
- **THEN** the motion controller receives `set_speed(0.0, 0.0)`, the drive remains in the running state, and the PID regulates both wheel speeds to zero instead of executing `power_stop()`

#### Scenario: Small non-zero command is not treated as zero

- **WHEN** the line `set_speed 0.04 0.04` arrives
- **THEN** it is applied as an ordinary non-zero setpoint without a zero-threshold rewrite

#### Scenario: Malformed line is ignored

- **WHEN** a line such as `set_speed abc def`, `stop_now`, or `set_speed 1 2 3` arrives
- **THEN** the line is ignored and the current command remains active

#### Scenario: Line terminator handling

- **WHEN** the parser receives a complete command terminated by a newline
- **THEN** the command is processed exactly once, and partial fragments are buffered until the newline

## ADDED Requirements

### Requirement: Parse explicit stop commands over Hardware Serial

The firmware SHALL parse the literal lines `stop` and `power_stop` as valid control commands. `stop` SHALL invoke the existing coasting `stop()` behavior; `power_stop` SHALL invoke the existing dynamic-braking `power_stop()` behavior. Both commands SHALL reset the command watchdog timer and SHALL be applied without changing the last commanded `set_speed` values, except that the telemetry echo becomes zero as specified by the telemetry capability. Lines with extra tokens, different spelling, or additional characters SHALL be ignored.

#### Scenario: stop command coasts

- **WHEN** the line `stop` arrives while the robot is moving
- **THEN** the motion controller executes `stop()`, the driver is disabled, and the wheels coast freely

#### Scenario: power_stop command brakes

- **WHEN** the line `power_stop` arrives while the robot is moving
- **THEN** the motion controller executes `power_stop()` and applies dynamic braking for the configured brake duration

#### Scenario: Stop commands keep the watchdog alive

- **WHEN** only `stop` or `power_stop` commands arrive at the heartbeat rate and no `set_speed` command arrives
- **THEN** the watchdog does not expire and the robot remains in the commanded stopped state

#### Scenario: Misspelled stop command is ignored

- **WHEN** a line such as `powerStop`, `stop_now`, or `stop extra` arrives
- **THEN** it is ignored without changing the current command or the watchdog state
