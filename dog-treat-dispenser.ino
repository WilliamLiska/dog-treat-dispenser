// Dog Treat Dispenser with Adafruit IO

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
int backAngle = 170;
int maxbackAngle = 175;
int forwardAngle = 108;
int minforwardAngle = 105;
bool potMode = false;

// set up the Treat Dispenser feed
AdafruitIO_Feed *treatdispenserfeed = io.feed("TreatDispenser");

// set up the DispenserOptions feed
AdafruitIO_Feed *dispenserOpts = io.feed("DispenserOptions");

//Vars for auto mode
bool autoMode = true;
unsigned long previousMillis = 0; //store the last time we dispensed a treat
long autoWaitTime = 600000; // Time to wait between auto dispenses

void setup() {
  
  pinMode(LED_PIN, OUTPUT);
  
  // start the serial connection
  Serial.begin(115200);

  // wait for serial monitor to open
  while(! Serial);

  // connect to io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  io.connect();

  // set up a message handler for the 'treatdispenserfeed' feed.
  // the handleMessage function (defined below)
  // will be called whenever a message is
  // received from adafruit io.
  treatdispenserfeed->onMessage(handleMessage);
  dispenserOpts->onMessage(handleOptionsMessage);

  // wait for a connection
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());
  treatdispenserfeed->get();

  // tell the servo class which pin we are using
  servo.attach(SERVO_PIN);
  servo.write(backAngle);
  Serial.print("Servo initialized to backAngle ");
  Serial.println(backAngle);
  
}

void loop() {

  // io.run(); is required for all sketches.
  // it should always be present at the top of your loop
  // function. it keeps the client connected to
  // io.adafruit.com, and processes any incoming data.
  io.run();

  if(potMode)
    potModeLoop();
  else if(autoMode){
    autoModeLoop();
  }
}

// Handles processing during pot mode.
void potModeLoop() {
  int reading = analogRead(potPin);
  int angle = map(reading, 1, 1024, forwardAngle, backAngle);
  servo.write(angle);
  Serial.println(reading);
  Serial.println(angle);
  delay(500);
}

// Handles processing during automatic mode.
void autoModeLoop() {
  unsigned long currentMillis = millis();

  if(currentMillis - previousMillis >= autoWaitTime){
    Serial.print("Requesting treat dispense at ");
    Serial.print(currentMillis);
    Serial.println(" ms.");
    requestDispenseTreat();
    previousMillis = currentMillis;  // Remember the time
  }
}

// this is called whenever a DispenserOption message is received.
void handleOptionsMessage(AdafruitIO_Data *data) {
  Serial.print("Option received <- ");
  Serial.println(data->toUnsignedInt());

  switch(data->toUnsignedInt()){
    case 5:
      Serial.println("Request to increase backAngle received");
      setbackAngle(backAngle + 5);
      break;
    case 13:
      Serial.println("Request to decrease backAngle received");
      setbackAngle(backAngle - 5);
      break;
    case 10:
      Serial.println("Request to increase forwardAngle received");
      setforwardAngle(forwardAngle + 5);
      break;
    case 8:
      Serial.println("Request to decrease forwardAngle received");
      setforwardAngle(forwardAngle - 5);
      break;
    default:
      break;
  }
}

// this function is called whenever an 'treatdispenserfeed' feed message
// is received from Adafruit IO. it was attached to
// the 'treatdispenserfeed' feed in the setup() function above.
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
  servo.write(forwardAngle);
  Serial.println(forwardAngle);
  delay(300);
  servo.write(backAngle);
  Serial.println(backAngle);
  digitalWrite(LED_PIN, HIGH);
}

// Send a message to the treatdispenserfeed indicating a treat should be dispensed.
// We could simply call dispenseTreat(), but then the feed history would not be updated.
// The feed should be the source of truth for dispensing treats.
void requestDispenseTreat() {
  //Send a value of 1 to the feed
  treatdispenserfeed->save(1);
}

// backAngle setter
void setbackAngle(int angle){
  if(angle > maxbackAngle){
    backAngle = maxbackAngle;
  }
  if(angle < forwardAngle)
    backAngle = forwardAngle + 1;
  else
    backAngle = angle;

  Serial.print("backAngle set to ");
  Serial.println(backAngle);
}

// forwardAngle setter
void setforwardAngle(int angle){
  if(angle < minforwardAngle){
    forwardAngle = minforwardAngle;
  }
  if(angle > backAngle)
    forwardAngle = backAngle - 1;
  else
    forwardAngle = angle;

  Serial.print("forwardAngle set to ");
  Serial.println(forwardAngle);
}
