/*
  This is an attempt to simulate mouth moving and body movement at the same time.
  Credit for initial source code is listed below. What is "new" in this version is
  the idea to detect when there is audio above a threshold (all sound is consider lyrics, 
  so yeah it is a little clunky that way).

  When sound is detected, it enters into a "mouth move" state which basically triggers
  an open mouth for a small random amount of time, then a close mouth for a small random
  amount of time. There is no delay to keep mouth open though, as the motor state is kept 
  "on" until the loop detects no sound. If sound is detected next time around, then repeat logic.

  Tail and Body movement work in a similar way but I try to toggle between "quarter", "half", "full".
  This bit of the code still needs some work.
  
   
  Initially this code was based off of instructables posting:
  https://www.instructables.com/id/Animate-a-Billy-Bass-Mouth-With-Any-Audio-Source/
  Make a DC Motor Move to Sound.
   This example code is in the public domain.
   Created by Donald Bell, Maker Project Lab (2016).
   Based on Sound to Servo by Cenk Özdemir (2012)
   and DCMotorTest by Adafruit
*/

// include the Adafruit motor shield library
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();

// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61);

/*
   Assign motor: The
*/
//motor_1 is for the body movement
// run(FORWARD) will toggle head
// run(BACKWARD) will activate tail
Adafruit_DCMotor *bodyMotor = AFMS.getMotor(1); //M1

//motor_2 will move the mouth: speed = 150 barely noticable, 200 half, 255 full
Adafruit_DCMotor *mouthMotor = AFMS.getMotor(2); //M2

boolean DEBUG_ON = false;

// Some other Variables we need
int SoundInPin = A0;
int LedPin = 12; //in case you want an LED to activate while mouth moves



/*
   When talking is determined, 
      schedule body and mouth movment for short durations
*/
bool talking;
long mouth_then;
long body_then;
long last_talked;

int body_speed;
int bodyDir = FORWARD;

/*
    The raw sensor values seem to stream in between 0 and 160 with my testing of a podcast.
    In the future, it might be nice to have a dynamic feature that is more sensative to volume
    settings.
*/
int MAX_SEEN;
int MIN_SEEN;

/*
   Reset the peak/min
*/
void resetSensorBoundary() {
  MAX_SEEN = 0;
  MIN_SEEN = 1600;
}

/*
  Monitor peaks and mins
*/
void updateSensorHistory(int rawSensorVal) {
  if (rawSensorVal > 4) {
    if (DEBUG_ON) {
      Serial.print("SensorValue: ");
      Serial.println(rawSensorVal);
    }

    if (MAX_SEEN < rawSensorVal ) {
      MAX_SEEN = rawSensorVal;
      if (DEBUG_ON) {
        Serial.print("New max observed: ");
        Serial.println(MAX_SEEN);
        delay(500);
      }
    }

    if ( MIN_SEEN > rawSensorVal ) {
      MIN_SEEN = rawSensorVal;
      if (DEBUG_ON) {
        Serial.print("New min observed: ");
        Serial.println( MIN_SEEN);
        delay(500);
      }
    }

  }
}


/*
   Setup routine runs once when you press reset:
*/
void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps

  AFMS.begin();  // create with the default frequency 1.6KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  bodyMotor->setSpeed(0); 
  bodyMotor->run(FORWARD);
  // turn on motor
  bodyMotor->run(RELEASE);

  mouthMotor->setSpeed(0); 
  mouthMotor->run(FORWARD);
  // turn on motor
  mouthMotor->run(RELEASE);

  pinMode(SoundInPin, INPUT);


  talking = false;

  //todo: resetSensorBoundary is probably not needed
  resetSensorBoundary();

  //schedule starting points
  mouth_then = millis();
  body_then = millis();
  last_talked = millis();
  body_speed = 255;
}

bool talk_state = false;


/*
     Main loop
*/
void loop() {

  /* Read the input on analog pin 0,
      my testing yielded values between 0 and 150ish,
      with normal talking at about 20ish to 60ish
      less than 12 was basically silent
  */
  int sensorValue = analogRead(SoundInPin);
  updateSensorHistory(sensorValue);

  /*
     This is an attempt to create a "state based"
     system where the fish is either talking or not talking.
  */
  int silentThreshold = 12;  //1 is arbiturary but basically silent
  if (sensorValue < silentThreshold && talking) {
    if (millis() > mouth_then) {
      talking = false;
      //schedule mouth to be closed for small amount of time
      //play with the random range for different effect
      mouth_then = millis() + floor(random(75, 150));
    }
  }
  else if (sensorValue > silentThreshold) {
    if (millis() > mouth_then) {
      talking = true;
      //play with the random range for different effect
      mouth_then = millis() + floor(random(70, 100));
    }
  }

  /*
     If we are within the currently scheduled window,
     act accordingly.
  */
  if (millis() < mouth_then) {
    if (talking) {
      openMouth();
      last_talked = millis();
      articulateBody(true);

    }
    else {
      shutMouth();
      articulateBody(false);
    }
  } else {
    talking = false;
  }


  /*
    Flap when it get's kinda silent for a moment
  */
  if (millis() - last_talked > 800) {
    last_talked = millis() + 60L * 60L * 1000L; //sleep for a little while
    bodyMotor -> run(RELEASE);
  }

  delay(10);
}


void flapTail(int times, int duration) {
  Serial.print("Flap Tail: ");
  Serial.println(times);
  for (int i = 0; i < times; i++) {
    bodyMotor -> run(BACKWARD);
    bodyMotor -> setSpeed(255);
    delay(duration);
    bodyMotor -> run(RELEASE);
    delay(duration);
  }
  bodyMotor -> run(RELEASE);
}


/*
 * No concrete logic here, just trying to hold a 
 * body position if we are talking but simulate a 
 * little movement for longer spans of talking... 
 *    still rough around the edges
 */
void articulateBody(bool talking) {
  bodyMotor -> run(bodyDir);
  if (talking) {
    //Hold body speed for 1.5 to 3 seconds
    if (millis() > body_then) {
      int r = floor(random(0, 8));
      Serial.print("r is:");
      Serial.println(r);
      if (r < 1) {
        body_speed = 0;
        body_then = millis() + floor(random(500, 1000));
        bodyDir = FORWARD;

      } else if (r < 3) {
        body_speed = 150;
        body_then = millis() + floor(random(500, 1000));
        bodyDir = FORWARD;

      } else if (r == 4) {
        body_speed = 200;
        body_then = millis() + floor(random(500, 1000));
        bodyDir = FORWARD;

      } else if ( r == 5 ) {
        body_speed = 0;
        bodyMotor -> run(RELEASE);
        delay(1);
        bodyDir = BACKWARD;
        bodyMotor -> setSpeed(255);
        delay(1);
        body_then = millis() + floor(random(900, 1200));
      }
      else {
        //bodyDir = FORWARD;
        body_speed = 255;
        body_then = millis() + floor(random(1500, 3000));
      }
    }

    bodyMotor -> setSpeed(body_speed);
  } else {
    if (millis() > body_then) {
      bodyMotor -> run(RELEASE);
      body_then = millis() + floor(random(20, 50));
    }
  }
}

void shutMouth( ) {
  mouthMotor ->run(RELEASE);
  mouthMotor ->setSpeed(0);
  mouthMotor ->run(BACKWARD);
  mouthMotor ->setSpeed(225);
  delay(1);
  mouthMotor ->setSpeed(0);
}

void openMouth( ) {
  mouthMotor ->run(FORWARD);
  mouthMotor ->setSpeed(100);
  delay(1);
  mouthMotor ->setSpeed(255);
}
