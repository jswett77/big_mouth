/*
   This is  test script based off of the internet sensation:
     https://www.instructables.com/id/Animate-a-Billy-Bass-Mouth-With-Any-Audio-Source/

*/

// include the Adafruit motor shield library
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield();


/*
   Assign motor
   bodyMotor is for the body movement (depends on the wiring)
   run(FORWARD) will toggle head
   run(BACKWARD) will activate tail
*/
Adafruit_DCMotor *bodyMotor = AFMS.getMotor(1); //M1

// mouthMotor will move the mouth:
// speed = 150 barely noticable, 200 half, 255 full
//   I noticed with batteries I could get more "action"
Adafruit_DCMotor *mouthMotor = AFMS.getMotor(2); //M2

//for print lines
boolean DEBUG_ON = true;

// Some other Variables we need
int SoundInPin = A0;

/*
    The raw sensor values seem to stream in between 0 and 160 with my testing of a podcast.
    In the future, it might be nice to have a dynamic feature that is more sensative to volume
    settings.
*/
int MAX_SEEN;
int MIN_SEEN;

/*
   When talking is determined, determine how long to mimic chatter

*/
bool talking;
long then, delta_t;
long body_then, body_delta_t;


int talk_level = 1;

bool BODY_ON = false;

/*
   Reset the peak/min
*/
void resetSensorBoundary() {
  MAX_SEEN = 0;
  MIN_SEEN = 1600;
}

/*
  Monitor peaks and mins..
    Attempting to see where my peaks and valleys are
    when sound is sent in at full volume from source out
     note, that yod don't actually need the speaker to be
     full volume
     If the source volume is reduced then the rawSensorValues
     are reduced too
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
  Serial.begin(9600);       // set up Serial library at 9600 bps


  AFMS.begin();            // create with the default frequency 1.6KHz

  // Set the speed to start, from 0 (off) to 255 (max speed)
  bodyMotor->setSpeed(0); //body motor
  bodyMotor->run(FORWARD);
  // turn on motor
  bodyMotor->run(RELEASE);

  mouthMotor->setSpeed(0); //mouth motor
  mouthMotor->run(FORWARD);
  // turn on motor
  mouthMotor->run(RELEASE);


  pinMode(SoundInPin, INPUT);

  talking = false;
  resetSensorBoundary();
}


void loop() {
  Serial.println("Mouth Open....");
  mouthMotor -> run(FORWARD);
  mouthMotor -> setSpeed(255);
  delay(2000);

  Serial.println("Mouth Close...");
  mouthMotor -> setSpeed(0);
  delay(300);

  Serial.println("Tail up...");
  bodyMotor -> run(BACKWARD);
  bodyMotor -> setSpeed(100);
  delay(500);
  bodyMotor -> setSpeed(200);
  delay(500);
  bodyMotor -> setSpeed(255);
  delay(500);
  Serial.println("Tail down...");
  bodyMotor -> setSpeed(0);

  flapTail(5, 100);

  gabGabGab(5);

  bobHead(5, 5000);
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
}


void bobHead(int times, int duration) {
  for (int i = 0; i < times; i++) {
    bodyMotor -> run(FORWARD);
    bodyMotor -> setSpeed(255);
    if (i % 2 == 0) {
      mouthMotor -> run(FORWARD);
      mouthMotor -> setSpeed(255);
    }
    delay(duration);
    bodyMotor -> run(RELEASE);
    mouthMotor -> run(RELEASE);
    delay(duration);
  }

}

void gab(int duration) {
  Serial.println("Gab...");
  mouthMotor -> run(RELEASE);
  mouthMotor -> setSpeed(0);
  delay(1);
  mouthMotor -> run(BACKWARD);
  mouthMotor -> setSpeed(255);
  delay(500);



  mouthMotor -> run(FORWARD);
  mouthMotor -> setSpeed(255);
  delay(duration);
}

void gabGabGab(int times) {
  for (int i = 0; i < times; i++) {
    gab(500);
    delay(10);
  }
}
