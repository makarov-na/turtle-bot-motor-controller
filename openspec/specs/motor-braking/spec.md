# Motor Braking

## Purpose
Define the safe default state of the motor driver after reset and the behavior of the `stop()` and `power_stop()` commands, so the robot never self-drives on boot and braking is explicit, bounded per call, and restartable by repeated calls.
## Requirements
### Requirement: Motors disabled after reset
On startup the controller SHALL leave the motors disabled so that no drive voltage is applied until a command is issued.

#### Scenario: Boot with motors off
- **WHEN** the controller boots
- **THEN** the motors remain off and the wheels can be turned by hand

### Requirement: stop() disables motors
`stop()` SHALL disable the motor driver so that the wheels coast freely with no braking force applied.

#### Scenario: Free coast
- **WHEN** `stop()` is called while the robot is stationary
- **THEN** the driver output is disabled and the wheels are free to spin

### Requirement: power_stop() applies dynamic braking
`power_stop()` SHALL apply the L298N fast motor stop (both IN inputs driven high while the enable is active) for the configured brake duration measured from the call, then release to the disabled state. Each call SHALL restart the brake timer.

#### Scenario: Brake then release
- **WHEN** `power_stop()` is called
- **THEN** both motors are actively braked for the configured brake duration and then released to the disabled state

#### Scenario: Repeated calls restart the bounded brake window
- **WHEN** `power_stop()` is called repeatedly at an interval shorter than the brake duration
- **THEN** each call restarts the brake timer, the driver stays in braking until one brake duration after the last call, and then releases to the disabled state

#### Scenario: power_stop heartbeat holds braking
- **WHEN** a `power_stop` command arrives every 200 ms while the brake duration is 300 ms
- **THEN** the driver remains in braking for as long as the commands continue, without coasting between frames
