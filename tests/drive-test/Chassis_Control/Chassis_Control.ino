#include <Servo.h>
#include <Wire.h>

// Insert own filepath here
#include "/home/collin/urc2014/tests/drive-test/Chassis_Control/DriveData.h"
// #include "C:\Users\Taylor\Documents\Arduino\libraries\DriveData.h"

//Packet size of DriveData object in bytes
#ifndef NECESSARY_PACKET_SIZE
#define NECESSARY_PACKET_SIZE  sizeof(DriveData)
#endif

#ifndef TIMEOUT
#define TIMEOUT 1000
#endif

const int TALON_LEFT = 9, TALON_RIGHT = 10, MIN_FREQUENCY = 1001, NEUTRAL_FREQUENCY = 1500, MAX_FREQUENCY = 2041;
unsigned long timeLastPacket;

Servo leftMotor, rightMotor;

DriveData data = { 0 };

inline void readBytes(void* dest, size_t len)
{
  for(char* charDest = (char*)dest; len > 0 && Wire.available(); --len, ++charDest )
    *charDest = Wire.read();
}

void i2cHandler(int numBytes)
{
  if(numBytes >= NECESSARY_PACKET_SIZE)
    timeLastPacket = millis();

  while(Wire.available() >= NECESSARY_PACKET_SIZE)
    readBytes(&data, sizeof(data));

  while(Wire.available() > 0)
    Wire.read();
}

void setup()
{
  // Attach the motors and set their starting speed to neutral/zero speed
  leftMotor.attach(TALON_LEFT);
  rightMotor.attach(TALON_RIGHT);
  leftMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
  rightMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
  
  Wire.begin(4);
  Wire.onReceive(i2cHandler);
  
  timeLastPacket = 0;
}

void loop()
{
  int leftSpeed = 0, rightSpeed = 0;
  
  leftSpeed = map(data.left_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
  rightSpeed = map(data.right_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
  
  if(millis() - timeLastPacket < TIMEOUT)
  {
    // Set speeds of motors and print
    leftMotor.writeMicroseconds(leftSpeed);
    rightMotor.writeMicroseconds(rightSpeed);
  }
  else
  {
    // Timeout handling (stop motors)
    leftMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
    rightMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
  }
  delay(3);
}
