## MODIFIED Requirements

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
