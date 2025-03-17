enum ParkingAndDoorState { // Always check DoorState then ParkingState
  STANDBY, // After five minute timeout, wait for something to happen.
  LEAVING, // Car present (ParkingState = NEAR or STOP AND DoorState = CLOSED changing to MOVING).  YELLOW LIGHT to GREEN when OPEN.  Once distance exceeds the nocarDistance, set ParkingAndDoorState to ARRIVING, resetting the parking mode.
  ARRIVING, // No car present (ParkingState = NOCAR AND DoorState = CLOSED changing to MOVING).  FLASHING YELLOW light to GREEN when OPEN.
  CLOSING // No car present since last STANDBY and door is closing.  YELLOW light to RED when CLOSED
  //OPENING // (Does not exist, same as ARRIVING) // No car present since last STANDBY and door is closing.  YELLOW light to GREEN when OPEN
};
