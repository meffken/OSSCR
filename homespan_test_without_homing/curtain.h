////////////////////////////////
//  control of the curtain    //
////////////////////////////////

const byte dirPin = 21;                               // pin connected to DIR pin of Easy tmc
const byte stepPin = 19;                              // pin connected to STEP pin of Easy tmc
const byte motorEnPin = 23;                           // motor sleep pin
const byte diagPin = 26;                              // diag pin from TMC 2209

const int movementSpeed = 4500;                       // motor speed
const int referenceSpeed = 1000;                      // motor speed for the reference run
const int movementAccel = 3500;                       // motor acceleration

const bool shaftDirection = true;                     // to reverse the motor direction

const uint8_t driverAddress = 0b00;                   // TMC2209 Driver address according to MS1 and MS2
const float rSense = 0.11f;                           // E_SENSE for current calc.
const unsigned int stallValue = 30;                   // [0..255] SGTHRS change if stall is wrong

unsigned long posEnd = 10000;                         // end position for testing set to a fixed position
unsigned long pos;                                    // current curtain position

bool stallGuard = false;

#include <AccelStepper.h>
#include <TMCStepper.h>

TMC2209Stepper driver(&Serial2, rSense, driverAddress);

AccelStepper stepper(1, stepPin, dirPin);             // 1 = Easy driver interface

void IRAM_ATTR diag() {
  stallGuard = true;
} 

struct DEV_curtain : Service::WindowCovering{

  SpanCharacteristic *curPos;                         // CurrentPosition      uint8_t
  SpanCharacteristic *tarPos;                         // TargetPosition       uint8_t
  SpanCharacteristic *posSta;                         // PositionState        uint8_t   | 0=closing | 1=opening | 2=stopped |
  //SpanCharacteristic *obsDet;                         // ObstructionDetected  boolean

  DEV_curtain() : Service::WindowCovering(){          // constructor() method
    
    curPos = new Characteristic::CurrentPosition(0);
    tarPos = new Characteristic::TargetPosition();
    tarPos->setRange(0, 100, 5);
    posSta = new Characteristic::PositionState();
    //obsDet = new Characteristic::ObstructionDetected(false);
    
  } // end constructor

  boolean update(){
    stepper.setMaxSpeed(movementSpeed);               // set movment speed
    stepper.setAcceleration(movementAccel);           // set acceleration  

    if(tarPos->updated()){
      if(tarPos->getNewVal() > curPos->getVal()){     // if the target-position requested is greater than the current-position  
        posSta->setVal(1);
        LOG1("Opening curtain\n");
      } else 
      if(tarPos->getNewVal() < curPos->getVal()){     // if the target-position requested is less than the current-position  
        posSta->setVal(0);
        LOG1("Closing  curtain\n");
      }
      
      driver.toff(4);                                 // enable motor
      pos = posEnd*(tarPos->getNewVal());             // calculate new target position
      pos = pos/100;

      stepper.moveTo(pos);                            // set new target positon
      stepper.runToPosition();                        // move stepper to the new position
      
      driver.toff(0);                                 // disable motor
      curPos->setVal(pos*100/posEnd);                 // set curtain position to homekit
      posSta->setVal(0);
      
      LOG1("The curtain has moved to position ");
      LOG1(curPos->getVal());
      LOG1("\n");
    }
    
    return(true);
  }
};


void stepper_reference_run() {
  Serial.println("Reference run !");
  attachInterrupt(diagPin, diag, RISING);             // activate interrupt
  
  driver.toff(4);                                     // enable motor
  
  while(stallGuard == false){
    driver.VACTUAL(-1000);
  }
  driver.VACTUAL(0);
  stallGuard = true;
  
  
  stepper.setCurrentPosition(-16);
  Serial.print("Reference run complete !");

  driver.toff(0);                                     // disable motor
  detachInterrupt(diagPin);                           // deactivate interrupt
}
