#include "Mode.h"
#include "DoorState.h"
#include "ParkingState.h"
#include "ParkingAndDoorState.h"
#include "Pattern.h"

// Settings
const int _nocarDistance = 450; // Centimeters.  About 15 feet
const int _nearDistance = 60; // About 2 feet
const int _stopDistance = 4; // About 2 inches

Mode mode = DOOR;

const float _updateInterval = 500.0; // 

const float flashRate = 1000.0;

// Pin setup
const int R_PIN = 6;
const int Y_PIN = 10;
const int G_PIN = 11;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Do not edit below this line
//
/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Traffic light display and effects
//
/////////////////////////////////////////////////////////////////////////////////////////////////
const int flashPeriodLength = floor(flashRate / _updateInterval);

// Clock for managing Patterns
int effectCounter = 0;

// Stores the on or off state of each color.
bool R = false;
bool Y = false;
bool G = false;

// Stores whether each color is solid on or flashing.
Pattern R_pattern = SOLID;
Pattern Y_pattern = SOLID;
Pattern G_pattern = SOLID;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Door and Parking Status
//
/////////////////////////////////////////////////////////////////////////////////////////////////
// State tracking
DoorState current_door_state = NULL;
DoorState from_door_state = NULL;
DoorState next_door_state = NULL;

ParkingState current_parking_state = NULL;
ParkingState from_parking_state = NULL;
ParkingState next_parking_state = NULL;

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
  Serial.println("Enter door state: O, C, M (Open, Closed, Moving): ");
  String input = Serial.readStringUntil('-');
  input.trim();
  if(input.length() > 0){
    if(input == "O"){
      return OPEN;
    }
    if(input == "C"){
      return CLOSED;
    }
    if(input == "M"){
      return MOVING;
    }
  }
  else {
    return current_door_state;
  }
}

// Sets the last state and next state for the door.
void UpdateDoorState(){
  current_door_state = GetDoorState();
  if(current_door_state == next_door_state){ // i.e., not OPEN or CLOSED.
    // reverse the states:
    next_door_state = from_door_state;
    from_door_state = current_door_state;
  }

}

// Determine what the light should be doing based on flash or solid patterns.
void ShowLight(){
  int flasher = floor(effectCounter / (flashPeriodLength / 2));

  bool this_R = R && (R_pattern == SOLID || (R_pattern == FLASH && flasher == 0));
  bool this_Y = Y && (Y_pattern == SOLID || (Y_pattern == FLASH && flasher == 0));
  bool this_G = G && (G_pattern == SOLID || (G_pattern == FLASH && flasher == 0));

  Serial.println("FLASHER: " + String(flasher ? "ON" : "OFF"));

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
//void ProcessDoorAndParking(){
//  //if(current_parking_state == STOP || current_parking_state == NEAR)
//  if(current_door_state == MOVING)
//}

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

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Initial conditions
//
/////////////////////////////////////////////////////////////////////////////////////////////////
void setup() {
  pinMode(R_PIN, OUTPUT);
  pinMode(Y_PIN, OUTPUT);
  pinMode(G_PIN, OUTPUT);
  
  digitalWrite(R_PIN, HIGH);
  digitalWrite(Y_PIN, HIGH);
  digitalWrite(G_PIN, HIGH);

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
}



/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Iterate
//
/////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
  // DOORANDPARKING
  UpdateDoorState();

//  if(mode == DOORANDPARKING){
//    ProcessDoorAndParking();
//  }
  if(mode == DOOR){
    ProcessDoor();
  }

  ShowLight();
  effectCounter = (effectCounter + 1) % flashPeriodLength;
  delay(_updateInterval);
  
}
