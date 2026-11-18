## MODIFIED Requirements

### Requirement: Encoder direction sign per wheel

The encoder SHALL report its measurement in the encoder's own counting direction, without applying the wheel's forward sign. Each wheel's motor SHALL apply the per-wheel sign when converting that measurement into the robot frame, so the reported wheel speed is positive when the wheel drives the robot forward, accounting for the mirror-mounted left motor. The sign SHALL be a property of the wheel assembly: the left and right wheel types SHALL each define their own sign, and no caller SHALL pass a sign on each measurement. The reported telemetry speeds, PWM outputs and integrals SHALL remain in the robot frame and SHALL keep their existing sign convention across this split.

#### Scenario: Forward rotation reads positive on both wheels
- **WHEN** the robot is commanded forward and both wheels rotate in the direction that moves the robot forward
- **THEN** both wheels' measured speeds are positive

#### Scenario: Backward rotation reads negative on both wheels
- **WHEN** the robot is commanded backward and both wheels rotate in the direction that moves the robot backward
- **THEN** both wheels' measured speeds are negative

#### Scenario: Encoder measurement is sign-free
- **WHEN** a wheel rotates in the encoder's positive counting direction
- **THEN** the encoder reports a positive measurement regardless of which side of the robot the wheel is mounted on

#### Scenario: Wheel sign is defined by the wheel type
- **WHEN** a left or right wheel motor is constructed
- **THEN** its forward sign comes from the wheel type, and no per-measurement sign argument is passed by the caller

#### Scenario: Telemetry sign convention is preserved
- **WHEN** both wheels drive the robot forward
- **THEN** the telemetry fields `mL`, `mR`, `oL` and `oR` are positive, and the integrals keep their previous robot-frame sign convention

#### Scenario: Sign correction without rewiring
- **WHEN** an encoder channel polarity mismatch causes a wheel to measure its direction inverted
- **THEN** the wheel's sign compensation can be flipped in firmware and the wheel then reads the correct direction
