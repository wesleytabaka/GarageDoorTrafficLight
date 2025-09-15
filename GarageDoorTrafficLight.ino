#include "Mode.h"
#include "DoorState.h"
#include "ParkingState.h"
#include "CarState.h"
#include "RunState.h"
#include "Pattern.h"

// Settings
const int _nocarDistance = 450; // Centimeters.  About 15 feet
const int _nearDistance = 60; // About 2 feet
const int _stopDistance = 4; // About 2 inches

const int cycleModeTiming[3] = {14200, 4200, 11200}; // G, Y, R
const int cycleModeMaxCycles = 3; // x GYR cycles then go into standby.

const int flashRedMaxCycles = 600;
const int flashYellowMaxCycles = 600;

Mode mode = DOORANDPARKING;

const float _updateInterval = 500.0; // 
const float _standbyInterval = 1500.0;

const float flashRate = 1000.0;

const float standbyTimeout = 60000.0; // Turn off after no changes.

// Pin setup
// Lamp pins
const int R_PIN = 6;
const int Y_PIN = 10;
const int G_PIN = 11;

// Door switch pins
const int CLOSED_PIN = 12;
const int OPEN_PIN = 13;

// Parking sensor pins
const int PARK_TRIG_PIN = 2;
const int PARK_ECHO_PIN = 3;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Do not edit below this line
//
/////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Bench testing variables
//
/////////////////////////////////////////////////////////////////////////////////////////////////
String lastInput = "";

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Traffic light display and effects
//
/////////////////////////////////////////////////////////////////////////////////////////////////
const int flashPeriodLength = floor(flashRate / _updateInterval);

// Clock for managing Patterns
int effectCounter = 0;

// Maintain the state of Cycle mode.
int cycleState = 0;
int cycleCycle = 0;

// Maintain the state of FlashRed mode.
int flashRedCycle = 0;

// Maintain the state of FlashYellow mode.
int flashYellowCycle = 0;

// Stores the on or off state of each color.
bool R = false;
bool Y = false;
bool G = false;

// Stores whether each color is solid on or flashing.
Pattern R_pattern = SOLID;
Pattern Y_pattern = SOLID;
Pattern G_pattern = SOLID;

float standbyTimeoutRemaining;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Door and Parking Status
//
/////////////////////////////////////////////////////////////////////////////////////////////////
// State tracking
RunState runstate = STANDBY;
CarState carstate = RESET;

DoorState current_door_state = NULL;
DoorState from_door_state = NULL;
DoorState next_door_state = NULL;
DoorState last_tick_door_state = NULL;

ParkingState current_parking_state = NULL;
ParkingState from_parking_state = NULL;
ParkingState next_parking_state = NULL;
ParkingState last_tick_parking_state = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Helper functions
//
/////////////////////////////////////////////////////////////////////////////////////////////////

// Set the colors and flash or solid.
void SetLight(bool new_R, bool new_Y, bool new_G, Pattern new_R_pattern, Pattern new_Y_pattern, Pattern new_G_pattern){
  R = new_R;
  Y = new_Y;
  G = new_G;
  R_pattern = new_R_pattern;
  Y_pattern = new_Y_pattern;
  G_pattern = new_G_pattern;
}

// Real world state from sensors.  This is implemented with serial input while on the bench and without real sensors.
DoorState GetDoorState(){ 
  // read pin for open
  // read pin for closed
  // decide what the state is and return it.
  bool doorOpen = digitalRead(OPEN_PIN) == LOW;
  bool doorClosed = digitalRead(CLOSED_PIN) == LOW;

  if(doorOpen && !doorClosed){
    return OPEN;
  }
  if(!doorOpen && doorClosed){
    return CLOSED;
  }
  if(!doorOpen && !doorClosed){
    return MOVING;
  }
}

float getParkingDistance(){
  digitalWrite(PARK_TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(PARK_TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(PARK_TRIG_PIN, LOW);

  float duration = pulseIn(PARK_ECHO_PIN, HIGH);
  float distance = (duration*.0343)/2;

  return distance;
}

// Real world state from sensors.  This is implemented with serial input while on the bench and without real sensors.
ParkingState GetParkingState(){
  float distance = getParkingDistance();

  if(distance >= _nearDistance){
    return NOCAR;
  }
  if(distance >= _stopDistance && distance < _nearDistance){
    return NEAR;
  }
  if(distance < _stopDistance){
    return STOP;
  }
}

// Sets the last state and next state for the door.
void UpdateDoorState(){
  last_tick_door_state = current_door_state;
  current_door_state = GetDoorState();
  if(current_door_state == next_door_state){ // i.e., not OPEN or CLOSED.
    // reverse the states:
    next_door_state = from_door_state;
    from_door_state = current_door_state;
  }
}

void UpdateParkingState(){
  last_tick_parking_state = current_parking_state;
  from_parking_state = current_parking_state;
  current_parking_state = GetParkingState();
  if(carstate == RESET){
    if(current_parking_state == NOCAR){
      carstate = ARRIVING;
    }
    else {
      carstate = LEAVING;
    }
  }
}

// Determine what the light should be doing based on flash or solid patterns and set the output pin levels.
void ShowLight(){
  int flasher = floor(effectCounter / (flashPeriodLength / 2));

  bool this_R = R && (R_pattern == SOLID || (R_pattern == FLASH && flasher == 0));
  bool this_Y = Y && (Y_pattern == SOLID || (Y_pattern == FLASH && flasher == 0));
  bool this_G = G && (G_pattern == SOLID || (G_pattern == FLASH && flasher == 0));


//  Serial.println(" _ ");
//  Serial.println(String("|\e[31m") + String(this_R ? "R" : " ") + String("\e[0m|"));
//  Serial.println(String("|\e[33m") + String(this_Y ? "Y" : " ") + String("\e[0m|"));
//  Serial.println(String("|\e[32m") + String(this_G ? "G" : " ") + String("\e[0m|"));
//  Serial.println(" ¯ ");
//  Serial.println("current_door_state: " + String(current_door_state));
//  Serial.println("from_door_state: " + String(from_door_state));
//  Serial.println("next_door_state: " + String(next_door_state));
  
  digitalWrite(R_PIN, this_R ? LOW : HIGH);
  digitalWrite(Y_PIN, this_Y ? LOW : HIGH);
  digitalWrite(G_PIN, this_G ? LOW : HIGH);

  
}


// Door and Parking mode only
void ProcessDoorAndParking(){
//  "Leaving"
//  C,S -> SR -- Same
  if(carstate == LEAVING){
  if(current_door_state == CLOSED){
    SetLight(true, false, false, SOLID, SOLID, SOLID);
  }
  
//  M,S -> SR -- Same
  if(current_door_state == MOVING && from_door_state == CLOSED){
    SetLight(true, false, false, SOLID, SOLID, SOLID);
  }
//  O,S -> SG -- need to know if the car was present at door activation time.
  if(current_door_state == OPEN){
    SetLight(false, false, true, SOLID, SOLID, SOLID);
  }
//  O,N -> SG -- need to know if the car was present at door activation time.
//  O,X -> SG -- Same
//  M,X -> SY -- need to know if the car was present at door activation time.
  if(current_door_state == MOVING && from_door_state == OPEN && current_parking_state == NOCAR){
    SetLight(true, false, false, SOLID, SOLID, SOLID);
  }
  if(current_door_state == MOVING && from_door_state == OPEN && (current_parking_state == NEAR || current_parking_state == STOP)){ // Door is moving as if about to leave, but car is still near.  Flash to get attention.
    SetLight(true, false, false, FLASH, SOLID, SOLID);
  }
//  C,X -> SR
//  STANDBY -> OFF
  }
//  
//  
//  "Arriving"
  if(carstate == ARRIVING){


//  C,X 
//  M,X -> FY
    if(current_door_state == MOVING && from_door_state == CLOSED){
      SetLight(false, true, false, SOLID, FLASH, SOLID);
    }
//  O,X -> SG
    if(current_door_state == OPEN && current_parking_state == NOCAR){
      SetLight(false, false, true, SOLID, SOLID, SOLID);
    }
//  O,N -> SY -- need to know if the car was present at door activation time.
    if(current_door_state == OPEN && current_parking_state == NEAR){
      SetLight(false, true, false, SOLID, SOLID, SOLID);
    }
//  O,S -> SR -- need to know if the car was present at door activation time.
    if(current_door_state == OPEN && current_parking_state == STOP){
      SetLight(true, false, false, SOLID, SOLID, SOLID);
    }
//    "Get out of car and close garage"
//  M,N or M,S -> SR
    if(current_door_state == MOVING && from_door_state == OPEN){
      SetLight(true, false, false, SOLID, SOLID, SOLID);
    }
//  C,N or C,S -> SR
    if(current_door_state == CLOSED && from_door_state == CLOSED){
      SetLight(true, false, false, SOLID, SOLID, SOLID);
    }
  }
//  STANDBY
}

// Door mode only
void ProcessDoor(){
  if(current_door_state == MOVING && from_door_state == CLOSED){
    SetLight(false, true, false, SOLID, FLASH, SOLID);
  }
  if(current_door_state == MOVING && from_door_state == OPEN){
    SetLight(false, true, false, SOLID, SOLID, SOLID);
  }
  if(current_door_state == OPEN){ 
    SetLight(false, false, true, SOLID, SOLID, SOLID);
  }
  if(current_door_state == CLOSED){ 
    SetLight(true, false, false, SOLID, SOLID, SOLID);
  }
}

void ProcessCycle(){
  switch(cycleState){
    case 0:
      SetLight(false, false, true, SOLID, SOLID, SOLID);
      break;
    case 1:
      SetLight(false, true, false, SOLID, SOLID, SOLID);
      break;
    case 2:
      SetLight(true, false, false, SOLID, SOLID, SOLID);
      cycleCycle++;
      break;
  }
  ShowLight();
  delay(cycleModeTiming[cycleState]);
  cycleState = (cycleState + 1) % 3;
  if(cycleCycle >= cycleModeMaxCycles){
    EnterStandby();
  }
}

void ProcessFlashRed(){
  SetLight(true, false, false, FLASH, SOLID, SOLID);
  ShowLight();
  flashRedCycle++;
  delay(_updateInterval);
  if(flashRedCycle >= flashRedMaxCycles){
    EnterStandby();
  }
}

void ProcessFlashYellow(){
  SetLight(false, true, false, SOLID, FLASH, SOLID);
  ShowLight();
  flashYellowCycle++;
  delay(_updateInterval);
  if(flashYellowCycle >= flashYellowMaxCycles){
    EnterStandby();
  }
}

void EnterStandby(){
  runstate = STANDBY;
  carstate = RESET;//NULL; // set "from" parking state to NULL.  or set carstate to NULL.
  cycleCycle = 0;
  flashRedCycle = 0;
  flashYellowCycle = 0;
  SetLight(false, false, false, SOLID, SOLID, SOLID); //  turn all lights off.
  ShowLight(); // Write to output pins.
}

void EnterWorking(){
  runstate = WORKING;
}

String getLampStateString(){
  return String("\[") + (R == true ? String("R") : String(" ")) + (R_pattern == SOLID ? String(" ") : String("*")) + " " + (Y == true ? String("Y") : String(" ")) + (Y_pattern == SOLID ? String(" ") : String("*")) + " " + (G == true ? String("G") : String(" ")) + (G_pattern == SOLID ? String(" ") : String("*")) + String("\]");
}

String getModeStateString(){
  switch(mode){
    case OFF:
      return "OFF           ";
      break;
    case DOORANDPARKING:
      return "DOORANDPARKING";
      break;
    case DOOR:
      return "DOOR          ";
      break;
    case PARKING:
      return "PARKING       ";
      break;
    case CYCLE:
      return "CYCLE         ";
      break;
    case FLASHRED:
      return "FLASHRED      ";
      break;
    case FLASHYELLOW:
      return "FLASHYELLOW   ";
      break;
  }
}

String getRunStateString(){
  switch(runstate){
    case WORKING:
      return "WORKING";
      break;
    case STANDBY:
      return "STANDBY";
      break;
  }
}

String getDoorStateString(){
  switch(current_door_state){
    case CLOSED:
      return "CLOSED   ";
      break;
    case MOVING:
      return "MOVING   ";
      break;
    case OPEN:
      return "OPEN     ";
      break;
  }
}

String getCarStateString(){
  switch(carstate){
    case RESET:
      return "RESET   ";
      break;
    case ARRIVING:
      return "ARRIVING";
      break;
    case LEAVING:
      return "LEAVING ";
      break;
  }
}

String getParkingStateString(){
  switch(current_parking_state){
    case NOCAR:
      return "NOCAR";
      break;
    case NEAR:
      return "NEAR ";
      break;
    case STOP:
      return "STOP ";
      break;
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Initial conditions
//
/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  runstate = STANDBY;
  
  pinMode(R_PIN, OUTPUT);
  pinMode(Y_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);

  pinMode(CLOSED_PIN, INPUT);
  pinMode(OPEN_PIN, INPUT);

  pinMode(PARK_TRIG_PIN, OUTPUT);
  pinMode(PARK_ECHO_PIN, INPUT);
  
  digitalWrite(R_PIN, HIGH);
  digitalWrite(Y_PIN, HIGH);
  digitalWrite(G_PIN, HIGH);

  digitalWrite(PARK_TRIG_PIN, LOW);

  Serial.begin(9600);
  Serial.setTimeout(0);
  
  current_door_state = GetDoorState();
  from_door_state = GetDoorState(); 
  if(current_door_state == OPEN){
    next_door_state = CLOSED;
  }
  if(current_door_state == CLOSED){
    next_door_state = OPEN;
  }

  standbyTimeoutRemaining = standbyTimeout;
}



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Iterate
//
/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  Serial.println(getLampStateString() + ", " + getModeStateString() + ", " + getRunStateString() + ", " + getDoorStateString() + ", " + getCarStateString() + ", " + getParkingStateString());
  UpdateDoorState();
  if(current_door_state != last_tick_door_state){ // This expression should be checking if the door state has changed.
    runstate = WORKING;
  }
  
  if(runstate == WORKING){
    if(mode == CYCLE){
      ProcessCycle();
    }

    if(mode == FLASHRED){
      ProcessFlashRed();
      effectCounter = (effectCounter + 1) % flashPeriodLength;
    }

    if(mode == FLASHYELLOW){
      ProcessFlashYellow();
      effectCounter = (effectCounter + 1) % flashPeriodLength;
    }
    
    if(mode == DOOR){
      ProcessDoor();
      ShowLight(); // 
      effectCounter = (effectCounter + 1) % flashPeriodLength;
      if(last_tick_door_state == current_door_state && standbyTimeoutRemaining > 0.0){
        standbyTimeoutRemaining -= _updateInterval;
      }
      else {
        standbyTimeoutRemaining = standbyTimeout; // Reset the standby timeout.
      }
      delay(_updateInterval);
    }

    if(mode == DOORANDPARKING){
      UpdateParkingState();
      ProcessDoorAndParking();
      ShowLight(); // 
      effectCounter = (effectCounter + 1) % flashPeriodLength;
      if(last_tick_door_state == current_door_state && last_tick_parking_state == current_parking_state && standbyTimeoutRemaining > 0.0){
        standbyTimeoutRemaining -= _updateInterval;
      }
      else {
        standbyTimeoutRemaining = standbyTimeout; // Reset the standby timeout.
      }
      delay(_updateInterval);
    }

    if(standbyTimeoutRemaining <= 0.0){
      EnterStandby();
    }

  }
  
  if(runstate == STANDBY){
    delay(_standbyInterval);
  }
}
