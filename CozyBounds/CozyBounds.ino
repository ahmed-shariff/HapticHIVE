#include "HapticHIVE.h"
#define SERIAL_BAUDRATE 115200
#define SMARTWATCH_REQUEST_DELAY 500 // decrease this delay (ms) to increase number of physiological readings per second

// TacHammer objects
extern TacHammer* M0;
extern TacHammer* M1;
extern TacHammer* M2;
extern TacHammer* M3;

// User variables
double i = 0;
double j = 0;
double k = 10;
int nextAnimationM0 = 0;
int nextAnimationM2andM3 = 0;
int nextAnimationM1andM2 = 0;
double currentHeartRate = 0;
double respiratoryRate = 0;

bool isPurring = false;
bool isGrowling = false;
double intensityM1 = 0.0;
double intensityM2 = 0.0;
double intensityM3 = 0.2;
bool isIncreasing = true; // Flag to track intensity increase or decrease

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
    Serial.print("[");
    Serial.print(time);
    Serial.print("] heart rate:");
    Serial.println(bpm);
    currentHeartRate = bpm; // save heartrate reading
    respiratoryRate = currentHeartRate/4; // rought estimation of the respiratory rate
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
    isGrowling = true;
    isPurring = false;
    Serial.println("Stressed stated started");
}

void buddyStressedStateUpdate() {
    // Code for growling animation
    unsigned long startTime = 0; // Start time of the current period
    unsigned long periodDuration = 5000; // Duration of the periodic pattern in milliseconds
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    double progress = static_cast<double>(elapsedTime) / periodDuration;
    intensityM1 = 0.5;
    intensityM2 = 0.5;

    // Repeatedly generate a vibration on M2 and M3
    if (isFree(M1) && isFree(M2)) {
        switch (nextAnimationM1andM2) {
        case 0:
            // Generate a vibration on M2 and M3 with randomized intensity changes
            vibrate(M1, 300, intensityM1, 0.5, 70);
            vibrate(M2, 160, intensityM2, 0.5, 70);

            nextAnimationM1andM2 = 1; // Set the next animation to run once M2 and M3 are free again
            break;
        case 1:
            if (isIncreasing) {
                // Increase the intensity periodically
                double increaseFactor = sin(progress * 2 * PI); // Adjust the periodic pattern here
                intensityM1 += increaseFactor * 0.05; // Adjust the increase factor here
                intensityM2 += increaseFactor * 0.05; // Adjust the increase factor here
            } else {
                // Decrease the intensity periodically
                double decreaseFactor = cos(progress * 2 * PI); // Adjust the periodic pattern here
                intensityM1 -= decreaseFactor * 0.05; // Adjust the decrease factor here
                intensityM2 -= decreaseFactor * 0.05; // Adjust the decrease factor here
            }
        
            // Cap the intensity values
            if (intensityM1 > 0.9) {
                intensityM1 = 0.9;
            }
            if (intensityM1 < 0.0) {
                intensityM1 = 0.0;
            }
            if (intensityM2 > 0.9) {
                intensityM2 = 0.9;
            }
            if (intensityM2 < 0.0) {
                intensityM2 = 0.0;
            }
        
            // Check if the current period has ended
            if (elapsedTime >= periodDuration) {
                startTime = currentTime; // Start a new period
                isIncreasing = !isIncreasing; // Toggle between increase and decrease
            }
        
            nextAnimationM1andM2 = 0; // Restart the sequence
            break;
        default:
            break;
        }
    }
}

void buddyRelaxedStateInit() {
    currentState = 2;
    stateStartTime = millis();
    isGrowling = false;
    isPurring = true;
    Serial.println("Relaxed state started");
}

void buddyRelaxStateUpdate() {
    unsigned long startTime = 0; // Start time of the current period
    unsigned long periodDuration = 5000; // Duration of the periodic pattern in milliseconds
    unsigned long currentTime = millis();
    unsigned long elapsedTime = currentTime - startTime;
    double progress = static_cast<double>(elapsedTime) / periodDuration;
    
    // Repeatedly generate a vibration on M2 and M3
    if (isFree(M2) && isFree(M3)) {
        switch (nextAnimationM2andM3) {
        case 0:
            // Generate a vibration on M2 and M3 with randomized intensity changes
            vibrate(M2, 110, intensityM2, 0.2, 70);
            vibrate(M3, 25, intensityM3, 0.2, 70);
          
            nextAnimationM2andM3 = 1; // Set the next animation to run once M2 and M3 are free again
            break;
        case 1:

            if (isIncreasing) {
                // Increase the intensity periodically
                double increaseFactor = sin(progress * 2 * PI); // Adjust the periodic pattern here
                intensityM2 += increaseFactor * 0.05; // Adjust the increase factor here
                intensityM3 += increaseFactor * 0.05; // Adjust the increase factor here
            } else {
                // Decrease the intensity periodically
                double decreaseFactor = cos(progress * 2 * PI); // Adjust the periodic pattern here
                intensityM2 -= decreaseFactor * 0.05; // Adjust the decrease factor here
                intensityM3 -= decreaseFactor * 0.05; // Adjust the decrease factor here
            }
          
            // Cap the intensity values
            if (intensityM2 > 0.15) {
                intensityM2 = 0.15;
            }
            if (intensityM2 < 0.0) {
                intensityM2 = 0.0;
            }
            if (intensityM3 > 0.3) {
                intensityM3 = 0.3;
            }
            if (intensityM3 < 0.0) {
                intensityM3 = 0.0;
            }
          
            // Check if the current period has ended
            if (elapsedTime >= periodDuration) {
                startTime = currentTime; // Start a new period
                isIncreasing = !isIncreasing; // Toggle between increase and decrease
            }
          
            nextAnimationM2andM3 = 0; // Restart the sequence
            break;
        default:
            break;
        }
    }
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

    
    // repeatedly generate a single pulse with intensity proportional to heart rate using M0
    if (isFree(M0)) { // if M0 is free
        if (currentHeartRate != 0) {
            switch (nextAnimationM0) { // check the next animation to run
            case 0: // animation n°1
                singlePulse(M0, .80, 8); // ask M0 to generate a single pulse at 50% intensity for 8ms
                nextAnimationM0 = 1; // set the next animation to run once M1 is free again
                break;
            case 1: // animation n°2
                if (isGrowling){
                    pause(M0, 60000 / (currentHeartRate + 50));
                }
                else {
                    pause(M0, 60000 / currentHeartRate); // ask M0 to pause itself for a time proportional to the heart rate
                }
                nextAnimationM0 = 0; // set the next animation to run once M0 is free again
                break;
            default:
                break;
            }
        }
    }

    isHelping = false;
}
