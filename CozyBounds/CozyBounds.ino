#include "HapticHIVE.h"
#define SERIAL_BAUDRATE 115200
#define SMARTWATCH_REQUEST_DELAY 500 // decrease this delay (ms) to increase number of physiological readings per second

// TacHammer objects
extern TacHammer* M0;
extern TacHammer* M1;
extern TacHammer* M2;
extern TacHammer* M3;

// User variables
unsigned long stateStartTime = 0;
// state = 0 - resting
// state = 1 - high stress
// state = 2 - stisfied
int currentState = 0;
bool isHelping = true; // This is set in either the gyro/accel/stepcount
bool needHelp = true; // TODO:Need to configure randomly at some point
// TODO: Will revisit second stage later
/* bool needHelpInStepTwo = true; // TODO:Need to configure randomly at some point */

float STEP_COUNT_THRESHOLD = 0.5 / 1000; // The threshold above which, it is considered to be helping
int STATE_WINDOW_SECONDS = 15; // The time window of a state

int stepsLastTime = 0;
double stepsLastCount = 0;


/**
 * Called whenever a new heart rate reading is received from the smartwatch. 
 * The function gives the time in ms at which the measurement was taken (*time*) and the heart rate in bpm (*bpm*).
 **/
void heartRateCallback(unsigned int time, double bpm) {
}

/**
 * Called whenever new accelerometer reading is received from the smartwatch.
 * The function gives the time in ms at which the measurement was taken (*time*) and the acceleration force along the X-axis (*xAccel*), Y-axis (*yAccel*), and the Z-axis (*zAccel*).
 **/
void accelerometerCallback(unsigned int time, double xAccel, double yAccel, double zAccel) {
//  Serial.print(time);
//  Serial.print(" ==== ");
//  Serial.print(xAccel);
//  Serial.print(" ==== ");
//  Serial.print(yAccel);
//  Serial.print(" ==== ");
//  Serial.print(zAccel);
//  Serial.println(" ==== ");
}

/**
 * Called whenever new gyroscope reading is received from the smartwatch. 
 * The function gives the time in ms at which the measurement was taken (*time*) and the rate of rotation in rad/s along the X-axis (*xRot*), Y-axis (*yRot*) and the Z-axis (*zRot*).
 **/
void gyroscopeCallback(unsigned int time, double xRot, double yRot, double zRot) {
}

/**
 * Called whenever a new light reading is received from the smartwatch.
 * The function gives the time in ms at which the measurement was taken (*time*) and the light measurement in lux (*lux*).
 **/
void lightCallback(unsigned int time, double lux) {
}

/**
 * Called whenever a new step count reading is received from the smartwatch. 
 * The function gives the time in ms at which the measurement was taken (*time*) and the number of steps since the smartwatch is started (*steps*).
 **/
void stepCounterCallback(unsigned int time, double steps) {
    if (stepsLastCount == 0) {
        stepsLastCount = steps;
        stepsLastTime = time;
        return;
    }
    isHelping = (steps - stepsLastCount)/ (time - stepsLastTime) > STEP_COUNT_THRESHOLD;
    Serial.print(steps);
    Serial.print("  ");
    Serial.print(time);
    Serial.print("  ");
    Serial.print(stepsLastCount);
    Serial.print("  ");
    Serial.print(stepsLastTime);
    Serial.print("  ");
    Serial.print(1000 * ((steps - stepsLastCount)/ (time - stepsLastTime)));
    Serial.print("  ");
    Serial.println(isHelping);

    stepsLastCount = steps;
    stepsLastTime = time;
}

// ================================================================================
void buddyRestStateInit() {
    needHelp = random(10) > 5;
    Serial.print("Rest state:  resting state - ");
    Serial.println(needHelp);
    currentState = 0;
    stateStartTime = millis();
}

void buddyRestStateUpdate() {
    
}

void buddyStressedStateInit() {
    currentState = 1;
    stateStartTime = millis();
    Serial.println("Stressed stated started");
}

void buddyStressedStateUpdate() {
    
}

void buddyRelaxedStateInit() {
    currentState = 2;
    stateStartTime = millis();
    Serial.println("Relaxed state started");
}

void buddyRelaxStateUpdate() {
    
}

// ================================================================================
// code to run once goes here
void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.setDebugOutput(true);
  setupTacHammers();
  setupSmartWatch(&heartRateCallback, &accelerometerCallback, &gyroscopeCallback, &lightCallback, &stepCounterCallback);

  buddyRestStateInit();
}

// ================================================================================
// active code goes here
void loop() { 
    requestSmartWatch(SMARTWATCH_REQUEST_DELAY); // requests new physiological readings from the smartwatch every SMARTWATCH_REQUEST_DELAY milliseconds

    if (millis() - stateStartTime > STATE_WINDOW_SECONDS * 1000) {
        switch (currentState) {
        case 0: {
            buddyStressedStateInit();
            break;
        }
        case 1: { // in stressed state
            if (!needHelp)
            {
                buddyRelaxedStateInit();
            } else {
                buddyStressedStateInit();
            }
            break;
        }
        case 2:{ // in relaxed state
            buddyRestStateInit();
            break;
        }
        default:
            break;
        }
    }

    switch (currentState) {
    case 0: {
        buddyRestStateUpdate();
        break;
    }
    case 1: { // in stressed state
        if (isHelping) {
            if (needHelp) {
                buddyRelaxedStateInit();
            } else {
                buddyStressedStateInit();
            }
        } else {
            buddyStressedStateUpdate();
        }
        break;
    }
    case 2:{ // in relaxed state
        buddyRelaxStateUpdate();
        break;
    }
    default:
        break;
    }

    isHelping = false;
}
