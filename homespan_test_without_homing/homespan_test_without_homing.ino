//-------------------------
//     ESP32 DEV Modul
//-------------------------

#include <AccelStepper.h>

#include "HomeSpan.h"

#include "curtain.h"
#include "battery.h"

const byte statusLED = 15;            // HomeSpan status LED

void setup() {
  Serial.begin(115200);
  Serial2.begin(500000);

  pinMode(motorEN, OUTPUT);           // motor sleep pin
  pinMode(batPin, INPUT);             // battery analg pin
  
  digitalWrite(motorEN, HIGH);
  
  digitalWrite(motorEN, LOW);         // enable motor
  stepper_reference_run();            // starts a reference run
  digitalWrite(motorEN, HIGH);        // disable motor

  //homeSpan.setControlPin(pin);        // set the control button
  homeSpan.setStatusPin(statusLED);   // set the status LED for HomeSpan
  homeSpan.setStatusAutoOff(60);      // sets Status LED to automatically turn off after duration seconds
  homeSpan.setSketchVersion("0.9.5"); // set the sketch version
  //homeSpan.setApSSID("");             // sets the SSID (network name) of the HomeSpan Setup Access Point
  //homeSpan.setApPassword("");         // sets the password of the HomeSpan Setup Access Point
  //homeSpan.setQRID("HSPN");           // changes the Setup ID(4), which is used for pairing a device with a QR Code, the HomeSpan default is "HSPN"
  //homeSpan.enableOTA(true, true);     // aktivate OTA funktion
  //homeSpan.enableAutoStartAP();
  //homeSpan.setPairingCode("46637726");
  homeSpan.setLogLevel(1);

  homeSpan.begin(Category::WindowCoverings, "HomeSpan OSSCR");

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Manufacturer("HomeSpan");
      new Characteristic::SerialNumber("321-test");
      new Characteristic::Model("OSSCR 2.0");
      new Characteristic::FirmwareRevision("0.9.5");

      new DEV_curtain();

      new DEV_battery();

}

void loop() {
  homeSpan.poll();

}