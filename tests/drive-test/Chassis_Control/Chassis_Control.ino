#include <Wire.h>
#include <Servo.h>
#include <map>
#include <string>

// Insert own filepath here
#include "C:\Users\Taylor\Documents\Arduino\libraries\ChassisData.h"

#ifndef I2C_ADDRESS
#define I2C_ADDRESS  1
#endif

#ifndef NECESSARY_PACKET_SIZE
#define NECESSARY_PACKET_SIZE  32 //Packet size of ChassisData object in bytes
#endif

#ifndef TIMEOUT
#define TIMEOUT 1000
#endif

const int TALON_LEFT = 13, TALON_RIGHT = 14, MIN_FREQUENCY = 1002, MAX_FREQUENCY = 2041;
unsigned long timeLastPacket;

Servo leftMotor, rightMotor;

ChassisData data;
boolean dataValid;

void I2CReceive(int count);
boolean readBytes(void* dest, size_t len);

void setup() {
  // Attach the motors and set their starting speed to neutral/zero speed
  leftMotor.attach(TALON_LEFT);
  rightMotor.attach(TALON_RIGHT);
  leftMotor.writeMicroseconds(1500);
  rightMotor.writeMicroseconds(1500);
  
  // Initialize the I2C Connections and specify the event listener I2CReceive
  Wire.begin(I2C_ADDRESS);
  Wire.onReceive(I2CReceive); 
  Serial.begin(115200);
  timeLastPacket = 0;
  dataValid = false;
  
  Serial.println("Iniialization complete.");
}

void loop() {
  static boolean timedOut = false;
  
  if(millis() - timeLastPacket < TIMEOUT && dataValid) {
    timedOut = false; 
    
    // Map values from 5-bit char to int from full reverse to full forwards
    int leftSpeed = map(data.left_motor, 0, 32, MIN_FREQUENCY, MAX_FREQUENCY);
    int rightSpeed = map(data.right_motor, 0, 32, MIN_FREQUENCY, MAX_FREQUENCY);
    
    // Set speeds of motors and print
    leftMotor.writeMicroseconds(leftSpeed);
    rightMotor.writeMicroseconds(rightSpeed);
    Serial.print("LEFT: ");
    Serial.println(leftSpeed);
    Serial.print("RIGHT: ");
    Serial.println(rightSpeed);
  } else {
    // Timeout handling (stop motors)
    leftMotor.writeMicroseconds(1500);
    rightMotor.writeMicroseconds(1500);
    if(!timedOut) {
      timedOut = true;
      Serial.println("TIMEOUT REACHED");
    }
  }
}

/**
  Receives and processes incoming I2C data
  Called as an event by Wire acting on request of an external I2C master
**/
void I2CReceive(int count) {
  if(count % NECESSARY_PACKET_SIZE == 0) {
    timeLastPacket = millis();
    
    for(int i = 0; i < count / NECESSARY_PACKET_SIZE && Wire.available() >= NECESSARY_PACKET_SIZE; i++) {
      readBytes(&data, NECESSARY_PACKET_SIZE);
      dataValid = true;
    }
  }
}

boolean readBytes(void* dest, size_t len) {
  unsigned char* tempDest = (unsigned char*)dest;
  for(; len > 0; --len) {
    if(Wire.available()){
      *tempDest = Wire.read();
      tempDest++;
    } else {
      return false;
    }
  }
  return true;
}
