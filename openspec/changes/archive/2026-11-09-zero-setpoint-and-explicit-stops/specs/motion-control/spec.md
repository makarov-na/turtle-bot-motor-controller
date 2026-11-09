## ADDED Requirements

### Requirement: Zero setpoint holds the wheels at rest

A `set_speed` command with both linear and angular velocity equal to zero SHALL be treated as a normal speed setpoint. The controller SHALL remain in the running closed-loop state and SHALL regulate both wheel speeds to zero. When a wheel's target changes from a non-zero value to exactly zero, the controller SHALL reset that wheel's PID integral once, so that stored windup cannot sustain motion. Repeated zero commands at the heartbeat rate SHALL NOT reset the integral again. The controller SHALL re-baseline its measurement reference when entering the running state after a pause, as before.

#### Scenario: Zero command keeps the loop running

- **WHEN** `set_speed(0, 0)` is applied while the robot is moving
- **THEN** both wheel targets are zero, the drive remains in the running state, the PID loop keeps executing, and the wheels are regulated to zero rather than switching to braking or coasting

#### Scenario: Stored windup does not sustain motion

- **WHEN** the robot has been running at a non-zero speed and then receives `set_speed(0, 0)`
- **THEN** the PID integral is reset once at that transition and the measured wheel speed converges to zero instead of settling at a non-zero equilibrium caused by the stored integral

#### Scenario: Repeated zero heartbeat preserves the integral

- **WHEN** `set_speed(0, 0)` arrives every 200 ms while the drive remains at zero target
- **THEN** the integral is not reset after the first transition, so the zero-hold loop can accumulate the small effort needed to keep the wheels at rest

#### Scenario: Zero hold resists external rotation

- **WHEN** the loop is running at zero target and a wheel is spun forward by hand
- **THEN** the measured speed is positive and the controller applies an opposing output instead of letting the wheel turn freely
