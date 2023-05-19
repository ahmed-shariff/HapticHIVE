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
int nextAnimationM1 = 0;
int nextAnimationM2andM3 = 0;
double currentHeartRate = 0;
double respiratoryRate = 0;

bool isPurring = false;
bool isGrowling = true;
double intensityM2 = 0.0;
double intensityM3 = 0.2;
bool isIncreasing = true; // Flag to track intensity increase or decrease

/**
   Called whenever a new heart rate reading is received from the smartwatch.
   The function gives the time in ms at which the measurement was taken (*time*) and the heart rate in bpm (*bpm*).
 **/
void heartRateCallback(unsigned int time, double bpm) {
  Serial.print("[");
  Serial.print(time);
  Serial.print("] heart rate:");
  Serial.println(bpm);
  currentHeartRate = bpm; // save heartrate reading
  respiratoryRate = currentHeartRate / 4; // rought estimation of the respiratory rate
}

/**
   Called whenever new accelerometer reading is received from the smartwatch.
   The function gives the time in ms at which the measurement was taken (*time*) and the acceleration force along the X-axis (*xAccel*), Y-axis (*yAccel*), and the Z-axis (*zAccel*).
 **/
void accelerometerCallback(unsigned int time, double xAccel, double yAccel, double zAccel) {
  // Serial.print("accelerometer: [");
  // Serial.print(xAccel);
  // Serial.print(", ");
  // Serial.print(yAccel);
  // Serial.print(", ");
  // Serial.print(zAccel);
  // Serial.println("]");
}

/**
   Called whenever new gyroscope reading is received from the smartwatch.
   The function gives the time in ms at which the measurement was taken (*time*) and the rate of rotation in rad/s along the X-axis (*xRot*), Y-axis (*yRot*) and the Z-axis (*zRot*).
 **/
void gyroscopeCallback(unsigned int time, double xRot, double yRot, double zRot) {
  // Serial.print("gyroscope: [");
  // Serial.print(xRot);
  // Serial.print(", ");
  // Serial.print(yRot);
  // Serial.print(", ");
  // Serial.print(zRot);
  // Serial.println("]");
}

/**
   Called whenever a new light reading is received from the smartwatch.
   The function gives the time in ms at which the measurement was taken (*time*) and the light measurement in lux (*lux*).
 **/
void lightCallback(unsigned int time, double lux) {
  // Serial.print("light:");
  // Serial.println(lux);
}

/**
   Called whenever a new step count reading is received from the smartwatch.
   The function gives the time in ms at which the measurement was taken (*time*) and the number of steps since the smartwatch is started (*steps*).
 **/
void stepCounterCallback(unsigned int time, double steps) {
  // Serial.print("step counter:");
  // Serial.println(steps);
}


// ================================================================================
// code to run once goes here
void setup() {
  Serial.begin(SERIAL_BAUDRATE);
  Serial.setDebugOutput(true);
  setupTacHammers();
  setupSmartWatch(&heartRateCallback, &accelerometerCallback, &gyroscopeCallback, &lightCallback, &stepCounterCallback);
}

// ================================================================================
// active code goes here
void loop()
{
  requestSmartWatch(SMARTWATCH_REQUEST_DELAY); // requests new physiological readings from the smartwatch every SMARTWATCH_REQUEST_DELAY milliseconds

  if (isGrowling) {
  // Code for growling animation
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
        vibrate(M1, 300, intensityM3, 0.2, 70);
        vibrate(M2, 160, intensityM2, 0.2, 70);
        
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
        if (intensityM2 > 0.25) {
          intensityM2 = 0.25;
        }
        if (intensityM2 < 0.0) {
          intensityM2 = 0.0;
        }
        if (intensityM3 > 0.5) {
          intensityM3 = 0.5;
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

  // repeatedly generate a single pulse with intensity proportional to heart rate using M0
  if (isFree(M1)) { // if M3 is free
    if (currentHeartRate != 0) {
      switch (nextAnimationM1) { // check the next animation to run
        case 0: // animation n°1
          if(isGrowling){
            singlePulse(M1 * 1.5, .70, 8); // ask M1 to generate a single pulse at 50% intensity for 8ms
          }
          else {
            singlePulse(M1, .70, 8); // ask M1 to generate a single pulse at 50% intensity for 8ms
          }
          nextAnimationM1 = 1; // set the next animation to run once M1 is free again
          break;
        case 1: // animation n°2
          pause(M1, 60000 / currentHeartRate); // ask M1 to pause itself for a time proportional to the heart rate
          nextAnimationM1 = 0; // set the next animation to run once M1 is free again
          break;
        default:
          break;
      }
    }
  }

  if (isPurring) {
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
          vibrate(M2, 120, intensityM2, 0.2, 70);
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
          if (intensityM2 > 0.25) {
            intensityM2 = 0.25;
          }
          if (intensityM2 < 0.0) {
            intensityM2 = 0.0;
          }
          if (intensityM3 > 0.5) {
            intensityM3 = 0.5;
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
}
