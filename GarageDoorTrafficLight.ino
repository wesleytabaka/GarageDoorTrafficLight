#include "Mode.h"
#include "DoorState.h"
#include "ParkingState.h"
#include "ParkingAndDoorState.h"
#include "Pattern.h"

// Settings
const int _nocarDistance = 450; // Centimeters.  About 15 feet
const int _nearDistance = 60; // About 2 feet
const int _stopDistance = 4; // About 2 inches

Mode mode = DOORANDPARKING;

int _updateInterval = 5000;

/////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Do not edit below this line
//
/////////////////////////////////////////////////////////////////////////////////////////////////

//Serial.begin(9600);

// Clock for managing Patterns
int patternclock = 0;

bool R = true;
bool Y = false;
bool G = false;
const int R_PIN = 6;
const int Y_PIN = 10;
const int G_PIN = 11;
Pattern R_pattern = SOLID;
Pattern Y_pattern = SOLID;
Pattern G_pattern = SOLID;

// State tracking
DoorState current_door_state = NULL;
DoorState from_door_state = NULL;
DoorState next_door_state = NULL;

ParkingState current_parking_state = NULL;
ParkingState from_parking_state = NULL;
ParkingState next_parking_state = NULL;

DoorState GetDoorState(){ // Real world state from sensors
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
}

void UpdateDoorState(){
  current_door_state = GetDoorState();
  if(current_door_state == next_door_state){ // i.e., not OPEN or CLOSED.
    // reverse the states:
    next_door_state = from_door_state;
    from_door_state = current_door_state;
  }

}

void setup() {
  current_door_state = GetDoorState();
  from_door_state = GetDoorState(); 
  if(current_door_state == OPEN){
    next_door_state = CLOSED;
  }
  if(current_door_state == CLOSED){
    next_door_state = OPEN;
  }
}

void ShowLight(){
  Serial.println(" _ ");
  Serial.println(String("|\e[31m") + String(R ? "R" : " ") + String("\e[0m|"));
  Serial.println(String("|\e[33m") + String(Y ? "Y" : " ") + String("\e[0m|"));
  Serial.println(String("|\e[32m") + String(G ? "G" : " ") + String("\e[0m|"));
  Serial.println(" ¯ ");
}

void loop() {
  // DOORANDPARKING
  UpdateDoorState();

  ShowLight();
  delay(_updateInterval);
}
