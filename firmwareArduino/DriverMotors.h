/*
 *  Motor control.
*/

#ifndef DRIVERMOTORS_H
#define DRIVERMOTORS_H

#include <Arduino.h>


#define RIGHT1              8 
#define SPEEDPINRIGHT       9 
#define RIGHT2              11
#define LEFT1               12
#define SPEEDPINLEFT        10
#define LEFT2               13

#define FINDER1             A0
#define FINDER2             A1
#define FINDER3             A2
#define FINDER4             A3
#define FINDER5             4       /*This pin is 4, not A4*/

#define MAXSPEED            255
#define MINSPEED            100
#define MINSPEED            0
#define MAXSPEEDREVERSE     -255

#define FORWARD             1
#define STOP                0
#define BACKWARDS           -1

#define print               Serial.print
#define println             Serial.println
#define PRINTDEBUG          print("\t[DEBUG] motordx: "); println(speedmotorRight);            \
                            print("\t[DEBUG] motorsx: "); println(speedmotorLeft);

//****************************************************************************************
//          Global variables
//****************************************************************************************

int speedmotorLeft;                     //Power of the left engine. This value will be given to the analogWrite function. The range is: [0:1:255]
int speedmotorRight;                    //Power of the right engine. This value will be given to the analogWrite function. The range is: [0:1:255]
int initValueMotorLeft;                 //Initial and minimum values of the engine. Is not possible to exceed these ranges of values
int initValueMotorRight;
int directionMotorLeft;
int directionMotorRight;


//****************************************************************************************
//          Prototypes
//****************************************************************************************

int readS1();
int readS2();
int readS3();
int readS4();
int readS5();
int getSpeed();
void goLeft();
void goRight();
void initialize(int min_speed_left, int min_speed_right);
void increaseSpeed(int powerOffset);
void decreaseSpeed(int powerOffset);	
void increaseSpeedLeft(int powerOffset);
void increaseSpeedRight(int powerOffset);
void decreaseSpeedLeft(int powerOffset);
void decreaseSpeedRight(int powerOffset);
void rightMotor(int direction);
void leftMotor(int direction);
void drive(int leftDirection, int rightDirection);
void forward();
void backwards();
void stop();

//****************************************************************************
//                  HIGH LEVEL FUNCTION
//****************************************************************************

void initialize(int min_speed_left, int min_speed_right){
    pinMode(RIGHT1,OUTPUT);
    pinMode(RIGHT2,OUTPUT);
    pinMode(SPEEDPINRIGHT,OUTPUT);
    pinMode(LEFT1,OUTPUT);
    pinMode(LEFT2,OUTPUT);
    pinMode(SPEEDPINLEFT,OUTPUT);
    pinMode(FINDER1,INPUT);
    pinMode(FINDER2,INPUT);
    pinMode(FINDER3,INPUT);
    pinMode(FINDER4,INPUT);
    pinMode(FINDER5,INPUT);
    directionMotorRight = directionMotorLeft = STOP;
    initValueMotorLeft  = speedmotorLeft  = min_speed_left;
    initValueMotorRight = speedmotorRight = min_speed_right;
}

int getSpeed() {
    if (speedmotorLeft != speedmotorRight)
        return -1;
    return speedmotorLeft;
}

void goLeft(){
    if (speedmotorRight< MINSPEED) speedmotorRight = initValueMotorRight;
    drive(STOP, FORWARD);
}

void goRight(){
    if (speedmotorLeft < MINSPEED) speedmotorLeft = initValueMotorLeft;
    drive(FORWARD, STOP);
}

void goForward() {
    if (speedmotorLeft < MINSPEED) speedmotorLeft = initValueMotorLeft;
    if (speedmotorRight< MINSPEED) speedmotorRight = initValueMotorRight;
    drive(FORWARD, FORWARD);
}

void goBackwards() {
    drive (BACKWARDS, BACKWARDS);
}

void stop() {
    drive (STOP, STOP);
}

void decreaseSpeed(int powerOffset){
    decreaseSpeedLeft(powerOffset);
    decreaseSpeedRight(powerOffset);
    drive(directionMotorLeft, directionMotorRight);
}

void increaseSpeed(int powerOffset){
    increaseSpeedLeft(powerOffset);
    increaseSpeedRight(powerOffset);
    drive(directionMotorLeft, directionMotorRight);
}
int readS1(){
    return digitalRead(FINDER1);
}

int readS2(){
    return digitalRead(FINDER2);
}

int readS3(){
    return digitalRead(FINDER3);
}

int readS4(){
    return digitalRead(FINDER4);
}

int readS5(){
    return digitalRead(FINDER5);
}



//****************************************************************************
//          LOW LEVEL FUNCTION
//****************************************************************************


void decreaseSpeedLeft(int powerOffset) {
    powerOffset %= MAXSPEED + 1;

    if (speedmotorLeft - powerOffset > initValueMotorLeft) {
        speedmotorLeft -= powerOffset;
    } else {
        speedmotorLeft = initValueMotorLeft;
    }
}

void decreaseSpeedRight(int powerOffset) { 
    powerOffset %= MAXSPEED + 1;

    if (speedmotorRight - powerOffset > initValueMotorRight) {
        speedmotorRight -= powerOffset;
    } else { 
        speedmotorRight = initValueMotorRight;
    }
}

void increaseSpeedLeft(int powerOffset) {
    powerOffset %= 256;

    if (powerOffset + speedmotorLeft < MAXSPEED) {
        speedmotorLeft += powerOffset;
    } else speedmotorLeft = 260;        //Trust me, is not an error. This makes some calculations easier
}

void increaseSpeedRight(int powerOffset) {
    powerOffset %= 256;
    if (powerOffset + speedmotorRight < MAXSPEED) {
        speedmotorRight += powerOffset;
    } else speedmotorRight = 260;
}


void drive(int left, int right) {
    directionMotorLeft = left;
    directionMotorRight = right;
    rightMotor(right);
    leftMotor(left);
}

/**
 * Value: 
 *  if FORWARD     ==> go forward
 *  if STOP        ==> stop engine
 *  if BACKWARDS   ==> go backwards
 */
void rightMotor(int direction) {
    if (direction == FORWARD) {
        analogWrite(SPEEDPINRIGHT, speedmotorRight >= MAXSPEED ? 255 : speedmotorRight);
        digitalWrite(RIGHT1, HIGH);
        digitalWrite(RIGHT2,LOW);//turn right motor clockwise
    } else if (direction == STOP) {
        analogWrite(SPEEDPINRIGHT, 0);
    } else {
        analogWrite(SPEEDPINRIGHT, speedmotorRight >= MAXSPEED ? 255 : speedmotorRight);
        digitalWrite(RIGHT1,LOW);
        digitalWrite(RIGHT2,HIGH); //turn right motor counterclockwise
    }
}

/**
 * Value: 
 *  if FORWARD     ==> go forward
 *  if STOP        ==> stop engine
 *  if BACKWARDS   ==> go backwards
 */
void leftMotor(int direction){
    if(direction == FORWARD) {
        analogWrite(SPEEDPINLEFT, speedmotorLeft >= MAXSPEED ? 255 : speedmotorLeft);
        digitalWrite(LEFT1,HIGH);
        digitalWrite(LEFT2,LOW);
    } else if(direction == STOP) {
        analogWrite(SPEEDPINLEFT, 0);
    } else { 
        analogWrite(SPEEDPINLEFT, speedmotorLeft >= MAXSPEED ? 255 : speedmotorLeft);
        digitalWrite(LEFT1,LOW);
        digitalWrite(LEFT2,HIGH);
    }
}
#undef print
#undef println 
#undef PRINTDEBUG
#endif
