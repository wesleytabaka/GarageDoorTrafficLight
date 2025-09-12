# Currently in development.
Currently works on the bench with serial inputs simulating sensors.
To test, send serial messages in the format:
G,P-

Where G is the Garage door state (M for MOVING, O for OPEN, C for CLOSED)
and P is the Parking state (X for NOCAR, N for NEAR, S for STOP).

E.g., "M,X-" means the garage door has started moving and no car is present.

# GarageDoorTrafficLight
Use an ATmega microcontroller to drive a traffic signal based on garage door position and parking sensors.

Garage door position sensors are controlled by Hall effect sensors read by microcontroller, one at top of door travel (DoorState.OPEN), one at end of door travel (DoorState.CLOSED).  When neither sensor is in range of door, it is considered to be (DoorState.MOVING).

Parking sensors are ultrasonic proximity sensors whose value is read by microcontroller.  Distance thresholds are adjustable.  There are three ParkingStates -- NOCAR (no car is present), NEAR (car is detected, but not "all the way up".  This state triggers the yellow lamp), and STOP (very close -- the car should stop.  This state triggers the red lamp).

Several "Modes" available.  Change traffic signal based on garage door alone, a combination of garage door and parking sensors, or just cycle between colors.

# Settings / Configuration
Within GarageDoorTrafficLight.ino, the following settings are configurable and are tested to work.  Modifications that you make below the "Do not edit below this line" are untested.

// Settings

const int _nocarDistance = 450; // Centimeters.  About 15 feet

const int _nearDistance = 60; // About 2 feet

const int _stopDistance = 4; // About 2 inches

const int cycleModeTiming[3] = {14200, 4200, 11200}; // G duration, Y duration, R duration in milliseconds.

const int cycleModeMaxCycles = 3; // Perform this number of GYR cycles then go into standby.

Mode mode = DOORANDPARKING; // The default mode.  E.g., when power is restored after a power loss, enter this state.

const float _updateInterval = 500.0; // In RunState.WORKING, how often to probe sensors or change lamp states.
const float _standbyInterval = 1500.0; // In RunState.STANDBY, how often to probe sensors.

const float flashRate = 1000.0; // How long a Pattern.FLASH cycle lasts in milliseconds.

const float standbyTimeout = 60000.0; // Turn lamps off and enter standby after no changes for this number of milliseconds.

const int R_PIN = 6; // Signal pin for red lamp.  LOW means the lamp is on.

const int Y_PIN = 10; // Signal pin for yellow lamp.  LOW means the lamp is on.

const int G_PIN = 11; // Signal pin for green lamp.  LOW means the lamp is on.
# API
## Mode (Enum)
- OFF: Manually power off the RYG lamps.
- DOORANDPARKING: Set the colored lamps based on the state of the garage door and the parking sensors.  
- DOOR: Set the colored lamps based on the state of the garage door only.
- PARKING: Not implemented.  Set the colored lamps based on the state of the parking sensors only.
- CYCLE: Cycle through lamps in the North American style, green for a long time, yellow for a few seconds, red for a long time.  Then repeat.  Great for kitschy restaurants.
- FLASHRED: Continuously flash red.
- FLASHYELLOW: Continuously flash yellow.

## Mode explanation
### OFF
Turns all lamps off.  Lamps remain off until a new mode is selected.

### DOORANDPARKING

"Arriving":
1. Garage door is closed and device is in "Standby".  No car is present in garage.  No lamps are on.  (CarState.ARRIVING)
2. Garage door begins to move to open.  Lamp is flashing yellow, indicating proceed with caution.
3. Garage door is fully open.  No car is present yet.  Lamp is solid green.
4. Car is detected.  Lamp is solid yellow.
5. Car is at the "stop" threshold.  Lamp is solid red.
6. You get out of car and door begins to move to close.  Lamp remains solid red.
7. After no change to door or parking state, device enters standby.  Lamps turned off.

"Leaving":
1. Garage door is closed and device is in "Standby".  Car is present in garage.  No lamps are on.  (CarState.LEAVING)
2. Garage door begins to move to open.  Lamp is solid red since car is present and door is not fully open.
3. Garage door is fully open.  Lamp is solid green, indicating okay to leave.
4. Car is no longer present.  Lamp remains solid green.
5. You use garage door remote to close door.  Lamp turns solid red indicating not to drive into or out of the garage while door is closing.
6. After no change to door or parking state, device enters standby.  Lamps turned off.

### DOOR
"Opening":
1. Garage door is closed and device is in "Standby".  No lamps are on.
2. Garage door begins to move.  Lamp is flashing yellow.
3. Garage door is fully open.  Lamp is solid green.
4. After no change to door state, device enters standby.  Lamps turned off.

"Closing":
1. Garage door is open and device is in "Standby".  No lamps are on.
2. Garage door begins to move.  Lamp is solid yellow.
3. Garage door is fully closed.  Lamp is solid red.

### PARKING
This mode is not implemented to prevent ultrasonic sensors running constantly.  Use DOORANDPARKING mode.

### CYCLE
1. Device is in "Standby".  No lamps are on.
2. User presses Mode select button, selecting in CYCLE mode.
3. Lamp is solid green for 14.2 seconds.
4. Lamp is solid yellow for 4.2 seconds.
5. Lamp is solid red for 11.2 seconds.
6. This cycle repeats for cycleModeMaxCycles times.

### FLASHRED
1. Device is in "Standby".  No lamps are on.
2. User presses Mode select button, selecting in FLASHRED mode.
3. Lamp flashes red for flashModeCycles times.

### FLASHYELLOW
1. Device is in "Standby".  No lamps are on.
2. User presses Mode select button, selecting in CYCLE mode.
3. Lamp flashes yellow for flashModeCycles times.

## DoorState (Enum)
- CLOSED
- MOVING
- OPEN

## ParkingState (Enum)
- NOCAR
- NEAR
- STOP

## CarState (Enum)
- RESET
- ARRIVING
- LEAVING

## RunState (Enum)
- STANDBY
- WORKING

## Pattern (Enum)
- SOLID
- FLASH

# Disclaimer
Use at your own risk.  Do not use this as a traffic signal controller.  Real traffic signal controllers are very robust and have failsafe features.  This does not.  I accept no liability for damages.  I'm just a guy who made a blinkenlights project.