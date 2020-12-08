// Open Source Smart Curtain Rod - Firmware v.0.01
//
// Make sure to fill in your Wifi login Data and your Adafruit iO Data
// Make sure you have used the same feeds name on Adafruit iO and in "Wifi Client" Section
// You can change the Travelling Speed and Acceleration in "AccelStepper Setup" Section if needed
// All Switches Normally Opened (NO) and connected to Ground
//
// HAVE FUN!
//-meffken



#include "AccelStepper.h" 
// Library created by Mike McCauley at http://www.airspayce.com/mikem/arduino/AccelStepper/

//Assistant Setup
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"

//Wifi login Data
#define WLAN_SSID       "XXXX"                                                                       //  +++Your SSID+++
#define WLAN_PASS       "XXXX"                                                                       //  +++Your password+++

//Adafruit.io Setup
#define AIO_SERVER      "io.adafruit.com"                                                            //  +++Adafruit Server+++
#define AIO_SERVERPORT  XXXX                   
#define AIO_USERNAME    "XXXX"                                                                       //  +++Username+++
#define AIO_KEY         "XXXX"                                                                       //  +++Auth Key+++

//WIFI CLIENT
WiFiClient client;
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);
Adafruit_MQTT_Subscribe Curtain1 = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/start_mqtt");    //  +++Feeds name should be same everywhere+++ in this case feeds name is "start_mqtt"

void MQTT_connect();

// AccelStepper Setup
AccelStepper stepper1(1, D5, D6);                             // 1 = Easy Driver interface
                                                              // Pin D5 connected to STEP pin of Easy Driver
                                                              // Pin D6 connected to DIR pin of Easy Driver
int movement_speed = 1500;                                    // Travelling Speed         (max value tested: 1500)-depends from stepper current/torque
int movement_accel = 1500;                                    // Travelling Acceleration  (max value tested: 1500)-depends from stepper current/torque

// Define the Pins used
const byte home_switch1 = D4;                                 // Pin D4 connected to Home Switch1 (MicroSwitch)
const byte home_switch2 = D3;                                 // Pin D3 connected to Home Switch2 (MicroSwitch)
const byte start_switch = D2;                                 // Pin D2 connected to Start Switch (currently not available)
const byte power = D1;                                        // Pin D1 connected to enable Pin of Stepper Driver

// Stepper Travel Variables
long initial_homing=-1;                                       // Used to Home Stepper at startup
long stepcounter;                                             // Used to count the Steps
int opened=1;                                                 // Used to check the status of the curtain (open/closed)
int finished=0;                                               // Used to check if the move is completed


void setup() {
   Serial.begin(9600);                                        // Start the Serial monitor with speed of 9600 Bauds
   
  // Define Inputs/Outputs
   pinMode(home_switch1, INPUT_PULLUP);
   pinMode(home_switch2, INPUT_PULLUP);
   pinMode(start_switch, INPUT_PULLUP);
   pinMode(power, OUTPUT);
   
  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());

  // Subscribe to MQTT
  mqtt.subscribe(&Curtain1);
   
  delay(5);                                                     // Wait for EasyDriver wake up

   // Set Max Speed and Acceleration of each Steppers at startup for homing
  stepper1.setMaxSpeed(5000.0);                                  // Set Max Speed of Stepper (Slower to get better accuracy)
  stepper1.setAcceleration(1000.0);                              // Set Acceleration of Stepper
 

  // Start Homing procedure of Stepper Motor at startup

  Serial.println("");
  Serial.println("");
  Serial.println("Find starting point");

  while (digitalRead(home_switch1)) {                           // Make the Stepper move CCW until the switch 1 is activated   
    stepper1.moveTo(initial_homing);                            // Set the position to move to
    initial_homing--;                                           // Decrease by 1 for next move if needed
    stepper1.run();                                             // Start moving the stepper 1 Step
    delay(50);
}

  Serial.println("Reached endstop 1");
  Serial.println("Driving out of endstop 1");
  initial_homing=1;
  
  while (!digitalRead(home_switch1)) {                          // Make the Stepper move CW until the switch 1 is deactivated
    stepper1.moveTo(initial_homing);                            // Set the position to move to
    initial_homing++;                                           // Increase by 1 for next move if needed
    stepper1.run();                                             // Start moving the stepper 1 Step
    delay(50);
  }
    
  stepper1.setCurrentPosition(0);                               // Reset Position
  Serial.println("Found starting point");
  Serial.println("");
  stepcounter = 0;                                              // Reset stepcounter
  delay(500);
  initial_homing=1;                                             // Reset initial_homing

  Serial.println("Find ending point");

  while (digitalRead(home_switch2)) {                           // Make the Stepper move CW until the switch 2 is activated
    stepper1.moveTo(initial_homing);                            // Set the position to move to
    initial_homing++;                                           // Increase by 1 for next move if needed
    stepper1.run();                                             // Start moving the stepper 1 Step
    stepcounter++;                                              // Count the steps moved
    delay(50);
  }

  Serial.println("Reached endstop 2");
  Serial.println("Driving out of endstop 2");
  
  stepper1.setCurrentPosition(0);                               // Set the current position as zero for now
  initial_homing=-1;

  while (!digitalRead(home_switch2)) {                          // Make the Stepper move CCW until the switch 2 is deactivated   
    stepper1.moveTo(initial_homing);                            // Set the position to move to
    initial_homing--;                                           // Decrease by 1 for next move if needed
    stepper1.run();                                             // Start moving the stepper
    stepcounter--;                                              // Count the steps moved
    delay(50);
   }

  Serial.println("Found ending point");
  Serial.println("");

  Serial.print("Traveled length:");
  Serial.println(stepcounter);
  Serial.println("Saved length");
  stepper1.setCurrentPosition(stepcounter);

  stepper1.setMaxSpeed(movement_speed);                          // Set Max Speed of Stepper (Faster for regular movements)
  stepper1.setAcceleration(movement_accel);                      // Set Acceleration of Stepper
  Serial.println("Waiting for commands");
  digitalWrite(power, HIGH);
  
}
  
void loop() {

  MQTT_connect();                                               // Connect to MQTT

  mqtt.ping();                                                  // Ping
  
  Serial.println(".");                                          // Write when pinging
  
  Adafruit_MQTT_Subscribe *subscription;

  if(!digitalRead(home_switch1)){
    Serial.println("Switching on Stepper");
    digitalWrite(power, LOW);                                                             // Switch Stepper Driver ON
    Serial.println("Driving out of endstop 1");
    stepper1.setCurrentPosition(0);                                                       // Reset Position
    stepper1.moveTo(50);                                                                  // Moving out of Endstop
    digitalWrite(power, HIGH);                                                            // Switch Stepper Driver OFF
  }

  if(!digitalRead(home_switch2)){
    Serial.println("Switching on Stepper");
    digitalWrite(power, LOW);                                                               // Switch Stepper Driver ON
    Serial.println("Driving out of endstop 2");
    stepper1.setCurrentPosition(stepcounter);                                               // Reset Position
    stepper1.moveTo(stepcounter-50);                                                        // Moving out of Endstop
    digitalWrite(power, HIGH);                                                              // Switch Stepper Driver OFF
  }
  
  while (subscription = mqtt.readSubscription(2000)) {

    if ((subscription == &Curtain1)&&(opened==1)&&(finished==0)) {                        // Close the curtain when its open and command is incoming
    Serial.print(F("Got: "));
    Serial.println((char *)Curtain1.lastread);
    int Curtain1_State = atoi((char *)Curtain1.lastread);
    digitalWrite(power, LOW);                                                             // Switch Stepper Driver ON
    Serial.println("Switching on Stepper");
    Serial.println("Driving OPEN");
    stepper1.moveTo(50);                                                                  // Set Position to move to
    opened=0;
    finished=1;
    }
      
    if ((subscription == &Curtain1)&&(opened==0)&&(finished==0)) {                        // Open the curtain when its closed and command is incoming
    Serial.print(F("Got: "));
    Serial.println((char *)Curtain1.lastread);
    int Curtain1_State = atoi((char *)Curtain1.lastread);
    digitalWrite(power, LOW);                                                             // Switch Stepper Driver ON
    Serial.println("Switching on tepper");
    Serial.println("Driving CLOSE");
    stepper1.moveTo(stepcounter-50);                                                      // Set Position to move to
    opened=1;
    finished=1;
    }
    }
    
    if ((finished == 1) && (stepper1.distanceToGo() == 0)) {                              // Switching OFF the Stepper Driver when move is finished
    digitalWrite(power, HIGH);
    //digitalWrite(D0, HIGH);                                                             // Toggle BuiltIn LED
    finished=0;
    Serial.println("Switching off stepper");
    }

    ESP.wdtDisable();                                                                           // Watchdog kicks in when movement is too long (maybe theres a better way to solve this...)
    while(stepper1.distanceToGo() != 0){                                                        // Loop for the movement
      stepper1.run();                                                                           // Do 1 Step
      if (((!digitalRead(home_switch1))||(!digitalRead(home_switch2)))&& finished==1) {         // Make sure to stop when endstop is reached
        digitalWrite(power, HIGH);                                                              // Switch OFF the Stepper Driver
        finished=0;
        Serial.println("Reached Endstop - switching stepper off");
      }
    }
    ESP.wdtEnable(1000); 
 

}

void MQTT_connect() {                                                                     // MQTT Connection
  int8_t ret;

  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  
  while ((ret = mqtt.connect()) != 0) {
    Serial.println(mqtt.connectErrorString(ret));
    Serial.println("Try to reconnect MQTT in 5s...");
    mqtt.disconnect();
    delay(5000); 
    retries--;
    if (retries == 0) {
      Serial.println("Could not connect to MQTT, starting 3 new attempts in 1min.");
      retries=3;
      delay(60000);
      //while (1);
    }
  }
  Serial.println("MQTT connected!");
  
}
