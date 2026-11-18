# Encoder Mapping

## Purpose
Define the mapping between motor encoders and wheel PID feedback channels, the per-wheel direction sign convention, and the wiring documentation so the robot drives straight and predictable.

## Requirements

### Requirement: Correct encoder-to-wheel mapping
The controller SHALL associate each wheel's PID feedback channel with the encoder physically attached to that same wheel. The left wheel SHALL read the encoder connected to pins A2/A3 and the right wheel SHALL read the encoder connected to pins A0/A1, matching the physical wiring.

#### Scenario: Left wheel feedback reads left encoder
- **WHEN** the left wheel's motor rotates and the right wheel's motor is stationary
- **THEN** the left wheel's measured speed reflects the left wheel's own encoder tick count and the right wheel's measured speed is zero

#### Scenario: Right wheel feedback reads right encoder
- **WHEN** the right wheel's motor rotates and the left wheel's motor is stationary
- **THEN** the right wheel's measured speed reflects the right wheel's own encoder tick count and the left wheel's measured speed is zero

#### Scenario: Straight forward motion
- **WHEN** `set_speed(250, 0)` is issued with the correct mapping in place
- **THEN** both wheels track their own commanded speed without the PID chasing the other wheel, and the robot does not turn sharply

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

### Requirement: Consistent wiring documentation
The wiring reference SHALL state the encoder connections that match the physical hardware: left motor encoder on A2/A3 and right motor encoder on A0/A1, consistent with the firmware mapping.

#### Scenario: Documentation matches hardware
- **WHEN** an operator rebuilds the wiring from the reference document
- **THEN** the encoder connections match the firmware mapping and the physical hardware
