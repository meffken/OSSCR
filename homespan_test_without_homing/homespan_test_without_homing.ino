////////////////////////////////
//      ESP32 DEV Modul       //
////////////////////////////////

#define curtain
#define blinds


#define homekit_code 46637726 // 46637726

#define auto_start_ap

#define homeSpan_ota

#define battery_mode


#define sketch_version "0.9.7"        // Software version
#define hardware_version "0.0.0"      // Hardware version

#include "HomeSpan.h"

#include "curtain.h"
#include "battery.h"

const byte statusLED = 15;            // HomeSpan status LED

void setup() {
  Serial.begin(115200);
  Serial2.begin(256000);

  pinMode(motorEnPin, OUTPUT);        // motor sleep pin
  pinMode(batPin, INPUT);             // battery analg pin

  digitalWrite(motorEnPin, LOW);      // enable motor in hardware
    
  driver.begin();
  driver.toff(0);                     // disable motor
  driver.blank_time(24);
  driver.rms_current(200); 
  driver.microsteps(8);
  driver.TCOOLTHRS(0x4E2);            // 20bit max //0xFFFFF
  //driver.seimin(1);
  driver.semin(0);//4
  driver.semax(2);//2
  driver.shaft(shaftDirection);
  driver.sedn(0b01);
  //driver.pwm_autoscale(true);
  driver.SGTHRS(stallValue);

  //stepper_reference_run();            // starts a reference run


  //homeSpan.setControlPin(pin);        // set the control button
  homeSpan.setStatusPin(statusLED);   // set the status LED for HomeSpan
  homeSpan.setStatusAutoOff(60);      // sets Status LED to automatically turn off after duration seconds
  homeSpan.setSketchVersion(sketch_version);  // set the sketch version
  //homeSpan.setApSSID("");             // sets the SSID (network name) of the HomeSpan Setup Access Point
  //homeSpan.setApPassword("");         // sets the password of the HomeSpan Setup Access Point
  //homeSpan.setQRID("HSPN");           // changes the Setup ID(4), which is used for pairing a device with a QR Code, the HomeSpan default is "HSPN"
  //homeSpan.enableOTA(true, true);     // aktivate OTA funktion, pw: "homespan-ota"
  //homeSpan.enableAutoStartAP();
  //homeSpan.setPairingCode("46637726");
  homeSpan.setLogLevel(1);            // for testing purpos

  homeSpan.begin(Category::WindowCoverings, "OSSCR");

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Manufacturer("HomeSpan");
      new Characteristic::SerialNumber("1123581321");
      new Characteristic::Model("OSSCR 2.0");
      new Characteristic::FirmwareRevision(sketch_version);
      new Characteristic::HardwareRevision(hardware_version);

      new DEV_curtain();

      new DEV_battery();

}

void loop() {
  homeSpan.poll();

}
