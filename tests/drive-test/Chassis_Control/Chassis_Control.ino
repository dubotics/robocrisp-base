#include <Servo.h>
#include <Wire.h>

// Insert own filepath here
#include "C:\Users\Taylor\Documents\Arduino\libraries\DriveData.h"

//Packet size of DriveData object in bytes
#ifndef NECESSARY_PACKET_SIZE
#define NECESSARY_PACKET_SIZE  sizeof(DriveData)
#endif

#ifndef TIMEOUT
#define TIMEOUT 1000
#endif

const int TALON_LEFT = 8, TALON_RIGHT = 9, MIN_FREQUENCY = 1001, NEUTRAL_FREQUENCY = 1500, MAX_FREQUENCY = 2041;
unsigned long timeLastPacket;

Servo leftMotor, rightMotor;

DriveData data = { 0 };
static char strbuf[128];

inline void readBytes(void* dest, size_t len)
{
  for(char* charDest = (char*)dest; len > 0 && Wire.available(); --len, ++charDest )
  {
    *charDest = Wire.read();
    char buf[4];
    //snprintf(buf, 4, "%02X ", *charDest);
    //Serial.print(buf);
  }
  //Serial.println();
}

void i2cHandler(int numBytes)
{
  while(Wire.available() >= NECESSARY_PACKET_SIZE)
  {
    //snprintf(strbuf, sizeof(strbuf), "DATA RECEIVED (%d): ", numBytes);
    //Serial.print(strbuf);
    readBytes(&data, sizeof(data));
    if(numBytes >= NECESSARY_PACKET_SIZE)
      timeLastPacket = millis();
    /*Serial.println("Left: ");
    Serial.println(data.left_motor);
    Serial.println("Right: ");
    Serial.println(data.right_motor);*/
  }
  /*while(Wire.available() > 0)
  {
    snprintf(strbuf, sizeof(strbuf), "Trash: %02X\n", Wire.read());
    Serial.print(strbuf);
  }*/
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
  
  Serial.begin(115200);
  timeLastPacket = 0;
  
  //Serial.println("Initialization complete.");
}

void loop()
{
  static bool timedOut = false;
  int leftSpeed, rightSpeed;
  
  leftSpeed = map(data.left_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
  rightSpeed = map(data.right_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
  
  Serial.println(millis() - timeLastPacket);
  
  if(millis() - timeLastPacket < TIMEOUT)
  {
    // Set speeds of motors and print
    leftMotor.writeMicroseconds(leftSpeed);
    rightMotor.writeMicroseconds(rightSpeed);
    timedOut = false;
  }
  else
  {
    // Timeout handling (stop motors)
    leftMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
    rightMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
    if(!timedOut) {
      timedOut = true;
      //Serial.println("TIMEOUT REACHED");
    }
  }
  delay(3);
}
