////////////////////////////////
//  control of the curtain    //
////////////////////////////////

const byte dirPin = 21;             // pin connected to DIR pin of Easy tmc
const byte stepPin = 19;            // pin connected to STEP pin of Easy tmc
const byte motorEN = 23;            // motor sleep pin

unsigned long posEnd = 10000;       // end position
const int movement_speed = 4500;    // motor speed
const int movement_accel = 3500;    // motor acceleration

unsigned long pos;                  // current curtain position


AccelStepper stepper(1, stepPin, dirPin); // 1 = Easy tmc interface
  
struct DEV_curtain : Service::WindowCovering{

  SpanCharacteristic *curPos;   // CurrentPosition      uint8_t
  SpanCharacteristic *tarPos;   // TargetPosition       uint8_t
  SpanCharacteristic *posSta;   // PositionState        uint8_t (2) ???
  //SpanCharacteristic *obsDet;   // ObstructionDetected  boolean

  DEV_curtain() : Service::WindowCovering(){  // constructor() method
    
    curPos = new Characteristic::CurrentPosition();
    tarPos = new Characteristic::TargetPosition();
    posSta = new Characteristic::PositionState();
    //obsDet = new Characteristic::ObstructionDetected(false);
    
  } // end constructor

  boolean update(){
    stepper.setMaxSpeed(movement_speed);              // set movment speed
    stepper.setAcceleration(movement_accel);          // set acceleration

    if(tarPos->getNewVal() != curPos->getVal()){      // if HomeKit request a position chanche
      digitalWrite(motorEN, LOW);                     // enable motor
      pos = posEnd*(tarPos->getNewVal());             // calculate new target position
      pos = pos/100;

      stepper.moveTo(pos);                            // set new target positon
      stepper.runToPosition();                        // move stepper to the new position
      
      digitalWrite(motorEN, HIGH);                    // disable motor
      curPos->setVal(pos*100/posEnd);                 // set curtain position to homekit
      Serial.print("Der Vorhang ist auf Position ");
      Serial.print(curPos->getVal());
      Serial.println(" gefahren");
    }

    return(true);
  }
};


void stepper_reference_run() {
  Serial.println("TODO: stepper_reference_run !!!!");
  stepper.setCurrentPosition(0);    // for test purpos
}
