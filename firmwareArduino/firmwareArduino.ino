/*  
 *  The goal of this firmware is to control the engines of the rover through sensors or via commands that arrive via serial.
    
 *  We have two main modalities
        AUTOMATIC. In this mode the sensors are read on board the rover and this will try to follow it.
        MANUAL. In this mode instead, rover receives commands from the serial and we don't care About the values of the sensors, therefore the sensors will not be checked.
    
 *  Technically we have another mode, that is STOPPED. Rover enters in this modality only if we were on the AUTOMATIC mode before.
        In this mode the rover locks the engines and waits to recive the CMD_RESTART command to restart in AUTOMATIC mode.
    
 *  In any of the three modes we find ourselves, it is always possible to receive the CHANGEMODALITY command, so we switch:
        If we are in AUTOMATIC mode, then go to MANUAL mode
        If we are in MANUAL mode, then go to AUTOMATIC mode
        If we are in STOPPED mode, ther go to MANUAL mode

        
 * MANUAL command (recived from users via serial)
     It is possible to make the rover go forward, go back, go left and go right.
     It is possible to modify the speed, at any time
        To go forward send key '8'
        To go backwards send key '2'
        To go right send key '6'
        To go left send key '4'

 * Modify speed
     Only in STOPPED mode it is not possible to modify the speed.
        To increase speed send key '7'
        To decrease speed send key '1'
*/
#include "DriverMotors.h"

#define SENSORS         loop1
#define MOTORS          loop2
#define COMMUNICATION   loop3


#define sprint          Serial.print
#define sprintln        Serial.println

//modes:
#define AUTOMATIC       '1'
#define MANUAL          '2'
#define STOPPED         '3'

//commands:
#define CHANGEMODALITY  'c'
#define CMD_STOP        '5'
#define RESTART         'r'

#define CMD_FORWARD     '8'
#define CMD_BACKWARDS   '2'
#define CMD_LEFT        '4'
#define CMD_RIGHT       '6'
#define CMD_MOTORUP     '7'
#define CMD_MOTORDOWN   '1'

//We decide to change the speed of these values. Range is: [0, 255]. Values outside this range will be changed automatically from "DriverMotors.h"
#define INCREASE_MOTOR  20
#define DECREASE_MOTOR  20


/**
 *  On "S1, S2, .., S5" we save the values read by the sensors.
 *  On "mode" the mode is saved
 *  On "actualCommand" we save the last command received
*/
static int S1, S2, S3, S4, S5;
static char mode;
static char actualCommand;



void setup() {
    Serial.begin(9600);
    arteLock();
    S1 = S2 = S3 = S4 = S5 = HIGH;
    arteUnlock();
    actualCommand = CMD_STOP;

    mode = getCommand();
    mode = mode == AUTOMATIC ? AUTOMATIC : MANUAL;
    initialize(140, 140);
}

/*
 * Read the serial, and stands still until we receive a command
*/
char getCommand() {
    char c;
    while (1){
        while (Serial.available()<=0);      //Block the execution
        c = Serial.read();
        if (c != '\n') break;
    }
    return c;
}

/*
 * This function moves the rover along the path (the path is a black line) and sends to the serial the direction in which it's going.
 * It tries to move in such a way the S3 sensor detects LOW, and the other sensors detect HIGH.
 * 
 *            SENSORS                                             Eg. Ope path                                     Eg. Close path
 *              ||                                                  . (END)                                        _______(START)/(END)
 *              S3                                                 /                                                 |       |
 *           S2 || S4                                              \__                                               |       |
 *         S1   ||   S5                                               \__.  (START)                                  |_______| 
 *
 * If the sensor don't detect any path (they don't find black under themselves) the rover will try to find a path backwards
 * This because:
 *  If we are in an open path (see above) then the rover will not run away, but will go back and forth cyclically
 *  If during a difficult curve the rover goes too far because it is going too fast, it will come back to find the lost line
*/
void follow_line() {
    int s1, s2, s3, s4, s5;
    arteLock();             //Start atomic section
    s1 = S1;
    s2 = S2;
    s3 = S3;
    s4 = S4;
    s5 = S5;
    arteUnlock();           //Finish atomic section

    if ((s3==LOW) && (s1==HIGH && s2==HIGH && s4==HIGH && s5==HIGH)){
        sprintln(CMD_FORWARD);
        goForward();
    } else {
        if (s1==LOW || s2==LOW) {
            sprintln (CMD_RIGHT);
            goRight();

        } else if (s5==LOW || s4==LOW){
            sprintln (CMD_LEFT);
            goLeft();

        } else if (s1 == HIGH && s2 == HIGH && s3 == HIGH && s4 == HIGH && s5 == HIGH) {
            goBackwards();
            sprintln (CMD_BACKWARDS);

        } else {
            Serial.print("We will never enter in this condition\n");
            Serial.print("-1\n");
            
            stop();
            while (1) delay(100);
        }
    }
}

/*
 * This function controls the rover
 * Sends the command executed on the serial line
*/
char driveRover(char cmd) {
    switch(cmd) {
        case CMD_FORWARD:
            sprintln (CMD_FORWARD);
            goForward();
            break;
        case CMD_RIGHT:
            sprintln (CMD_RIGHT);
            goRight();
            break;
        case CMD_LEFT:
            sprintln (CMD_LEFT);
            goLeft();
            break;
        case CMD_BACKWARDS:
            sprintln (CMD_BACKWARDS);
            goBackwards();
            break;
        case CMD_MOTORUP:
            sprintln (CMD_MOTORUP);
            increaseSpeed(INCREASE_MOTOR);
            return ' ';
        case CMD_MOTORDOWN:
            sprintln (CMD_MOTORDOWN);
            decreaseSpeed(DECREASE_MOTOR);
            return ' ';
        case CMD_STOP:
            sprintln (CMD_STOP);
            stop();
            break;
        default:
            sprint("[driveRover] default: <"); sprint(cmd); sprint("#>\n");
            return ' ';
    }
    return cmd;
}

/* 
 * This function is only called if we are in MANUAL mode. Sends via serial the current command that is running
*/
void sendAction(char cmd) {
    switch(cmd) {
        case CMD_FORWARD:
            sprintln (CMD_FORWARD);
            break;
        case CMD_RIGHT:
            sprintln (CMD_RIGHT);
            break;
        case CMD_LEFT:
            sprintln (CMD_LEFT);
            break;
        case CMD_BACKWARDS:
            sprintln (CMD_BACKWARDS);
            break;
        case CMD_STOP:
            sprintln (CMD_STOP);
            break;
        default:    /*Not verifiable*/
            sprintln("-1");
    } 
}


/*
 * Read and store the values of sensors
 */
void SENSORS(50) {
    if (mode == AUTOMATIC) {
        arteLock();               //Start atomic section
        S1 = readS1();
        S2 = readS2();
        S3 = readS3();
        S4 = readS4();
        S5 = readS5();
        arteUnlock();             //Finish atomic section
    }
}

/*
 * Thread MOTORS: change its action based on the mode
 * It must be able to move the rover only if we are in AUTOMATIC mode or MANUAL mode, else keep still the rover. 
 */
void MOTORS(100) {
    char cmd;
    switch (mode) {
        case STOPPED:
            stop();
            sprint(CMD_STOP);
            break;
        case AUTOMATIC:
            follow_line();
            break;
        case MANUAL:
            arteLock();
            cmd = actualCommand;
            arteUnlock();
            sendAction(cmd);
            break;
        default:   /*Not verifiable*/
            sprintln("-1");
    }
}

/* 
 * This task receive command from the serial and will call the corresponding management routine
 */
void COMMUNICATION(200) {
    char cmd = getCommand();
    switch (mode) {
        case AUTOMATIC:
            if      (cmd == CMD_STOP) {
                mode = STOPPED;

            } else if (cmd == CHANGEMODALITY) {
                mode = MANUAL;
                stop();

            } else if (cmd == CMD_MOTORUP) {
                increaseSpeed(20);

            } else if (cmd == CMD_MOTORDOWN) {
                decreaseSpeed(20);

            }
            break;
        case MANUAL:
            if (cmd == CHANGEMODALITY) {
                mode = AUTOMATIC;
            } else {
                cmd = driveRover(cmd);
                if (cmd != ' ') {
                    arteLock();                 //Start atomic section
                    actualCommand = cmd;
                    arteUnlock();               //Finish atomic section
                }
            }
            break;
        case STOPPED:
            if (cmd == CHANGEMODALITY)
                mode = MANUAL;
            else if (cmd == RESTART)
                mode = AUTOMATIC;
            break;
        default: /*Not verifiable*/
            sprintln("-1");
    }
}

void loop() {
}
