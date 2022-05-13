////////////////////////////////
//     batery management      //
////////////////////////////////

const byte batPin = 32;             // battery analg voltage pin

const int analogPinbytes = 4095;    // max read of the analogRead
int batPer;                         // percentage of the battery

struct DEV_battery : Service::BatteryService{

  SpanCharacteristic *batLev;       // uint8_t (0) (0 to 100)
  SpanCharacteristic *chaSta;       // uint8_t (0) (0 to 2) |0=no|1=yes|2=not chargeable| ?? 2 dint work ??
  SpanCharacteristic *StaLowBat;    // uint8_T (0) (0 to 1)

  DEV_battery() : Service::BatteryService(){  // constructor() method
    
    batLev = new Characteristic::BatteryLevel();
    chaSta = new Characteristic::ChargingState(2);
    StaLowBat = new Characteristic::StatusLowBattery();
    
  } // end constructor
  
  //boolean update(){
  //  return(true);
  //}
  
  void loop(){
    
    if(batLev->timeVal() > 30000){                    // update the battery every 30 min
      batPer = analogRead(batPin)*100/analogPinbytes; // calculate the battery percentage
      batLev->setVal(batPer);                         // set the battery percentage in HomeKit

      if(batPer < 30){                                // if batPer is below 30
        StaLowBat->setVal(1);                         // set low battery warning in HomeKit
      }else{
        StaLowBat->setVal(0);
      }
    }
  }
  
};
