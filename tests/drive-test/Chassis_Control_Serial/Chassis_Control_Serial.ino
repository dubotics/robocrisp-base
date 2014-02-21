#include <Servo.h>
#include <map>
#include <string>

// Insert own filepath here
#include "C:\Users\Taylor\Documents\Arduino\libraries\ChassisData.h"

//Packet size of ChassisData object in bytes
#ifndef NECESSARY_PACKET_SIZE
#define NECESSARY_PACKET_SIZE  sizeof(ChassisData)
#endif

#ifndef TIMEOUT
#define TIMEOUT 1000
#endif

const int TALON_LEFT = 8, TALON_RIGHT = 9, MIN_FREQUENCY = 1001, NEUTRAL_FREQUENCY = 1500, MAX_FREQUENCY = 2041;
unsigned long timeLastPacket;

Servo leftMotor, rightMotor;

ChassisData data;
boolean dataValid;

boolean readBytes(void* dest, size_t len);

void setup()
{
  // Attach the motors and set their starting speed to neutral/zero speed
  leftMotor.attach(TALON_LEFT);
  rightMotor.attach(TALON_RIGHT);
  leftMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
  rightMotor.writeMicroseconds(NEUTRAL_FREQUENCY);
  
  Serial.begin(115200);
  timeLastPacket = 0;
  dataValid = false;
  
  Serial.println("Iniialization complete.");
}

void loop()
{
  static boolean timedOut = false;
  static unsigned char fullByte = 255;
  int leftSpeed, rightSpeed;
  
  while(Serial.available() > NECESSARY_PACKET_SIZE + 1 || (Serial.peek() != fullByte && Serial.available() > 0))
  {
    int8_t i = Serial.read();
    Serial.print("Trashed: ");
    Serial.println(i);
  }
  
  if(Serial.available() == /*2*/ NECESSARY_PACKET_SIZE + 1 && Serial.peek() == fullByte)
  {
    Serial.read();
    /*int8_t i = Serial.read();
    Serial.println(i);*/
    timedOut = false;
    Serial.println("Data recieved.");
    //Serial.readBytes((char*)&data, NECESSARY_PACKET_SIZE);
    data.left_motor = (int8_t)Serial.read();
    data.right_motor = (int8_t)Serial.read();
    timeLastPacket = millis();
    dataValid = true;
    
    Serial.print("LEFT Data: ");
    Serial.println(data.left_motor);
    Serial.print("RIGHT Data: ");
    Serial.println(data.right_motor);
    
    // Map values from 5-bit char to int from full reverse to full forwards
    int leftSpeed = map(data.left_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
    int rightSpeed = map(data.right_motor, -127, 128, MIN_FREQUENCY, MAX_FREQUENCY);
    
    Serial.print("LEFT Speed: ");
    Serial.println(leftSpeed);
    Serial.print("RIGHT Speed: ");
    Serial.println(rightSpeed);
  }
  
  if(millis() - timeLastPacket < TIMEOUT && dataValid)
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
    if(!timedOut) {
      timedOut = true;
      Serial.println("TIMEOUT REACHED");
    }
  }
}
