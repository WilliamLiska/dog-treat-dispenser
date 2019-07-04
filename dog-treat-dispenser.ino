// Adafruit IO Digital Output Example
// Tutorial Link: https://learn.adafruit.com/adafruit-io-basics-digital-output
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include <Servo.h> 

#define LED_PIN 0

// pin used to control the servo
#define SERVO_PIN 2

// pin used to read pot position
int potPin = A0;

Servo servo;

int angle = 0;
int openAngle = 170;
int maxOpenAngle = 179;
int closedAngle = 31;
int minClosedAngle = 30;
bool potMode = true;

// set up the Treat Dispenser feed
AdafruitIO_Feed *digital = io.feed("TreatDispenser");

// set up the DispenserOptions feed
AdafruitIO_Feed *dispenserOpts = io.feed("DispenserOptions");

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  
  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'digital' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  digital->onMessage(handleMessage);
  dispenserOpts->onMessage(handleOptionsMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  digital->get();

  // tell the servo class which pin we are using
  servo.attach(SERVO_PIN);

}

void loop() {

  if(potMode)
    potModeLoop();
  else {
    // io.run(); is required for all sketches.
    // it should always be present at the top of your loop
    // function. it keeps the client connected to
    // io.adafruit.com, and processes any incoming data.
    io.run();
  }
}

void potModeLoop() {
  int reading = analogRead(potPin);
  int angle = map(reading, 1, 1024, closedAngle, openAngle);
  servo.write(angle);
  Serial.println(reading);
  Serial.println(angle);
  delay(500);
}

// this is called whenever a DispenserOption message is received.
void handleOptionsMessage(AdafruitIO_Data *data) {
  Serial.print("Option received <- ");
  Serial.println(data->toUnsignedInt());

  switch(data->toUnsignedInt()){
    case 5:
      Serial.println("Request to increase openAngle received");
      setOpenAngle(openAngle + 5);
      break;
    case 13:
      Serial.println("Request to decrease openAngle received");
      setOpenAngle(openAngle - 5);
      break;
    case 10:
      Serial.println("Request to increase closedAngle received");
      setClosedAngle(closedAngle + 5);
      break;
    case 8:
      Serial.println("Request to decrease closedAngle received");
      setClosedAngle(closedAngle - 5);
      break;
    default:
      break;
  }
}

// this function is called whenever an 'digital' feed message
// is received from Adafruit IO. it was attached to
// the 'digital' feed in the setup() function above.
void handleMessage(AdafruitIO_Data *data) {

  Serial.print("received <- ");

  if(data->toPinLevel() == HIGH){
    Serial.println("HIGH");
    dispenseTreat();
  }
  else
    Serial.println("LOW");

}

// responsible for dispensing a number of treats
void dispenseTreat(){
  digitalWrite(LED_PIN, LOW);   //Indicator light tell us when we're operating
  servo.write(openAngle);
  Serial.println(openAngle);
  delay(300);
  servo.write(closedAngle);
  Serial.println(closedAngle);
  digitalWrite(LED_PIN, HIGH);
}

void setOpenAngle(int angle){
  if(angle > maxOpenAngle){
    openAngle = maxOpenAngle;
  }
  if(angle < closedAngle)
    openAngle = closedAngle + 1;
  else
    openAngle = angle;

  Serial.print("openAngle set to ");
  Serial.println(openAngle);
}

void setClosedAngle(int angle){
  if(angle < minClosedAngle){
    closedAngle = minClosedAngle;
  }
  if(angle > openAngle)
    closedAngle = openAngle - 1;
  else
    closedAngle = angle;

  Serial.print("closedAngle set to ");
  Serial.println(closedAngle);
}
