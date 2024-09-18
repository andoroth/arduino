#include <Wire.h>
uint8_t address;
uint8_t data[3];
uint8_t signalBits;
uint16_t rawTemp, rawRH;
double temp, humidity;

void setup() {
  // put your setup code here, to run once:
  // Clock - pin A4, Data - pin A5, Slave address - 0x27
  // minClock = 100kHz
  // setClock(uint32_t frequency)
  Serial.begin(9600);
  address = 0x27;
  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  // put your main code here, to run repeatedly:
  //measurement request - slave address:R/W=0
  //Data fetch - slave address:R/W=1
  //Data format - 2bitstatus:6bitHumidity; 8bHumidity; 8b temp; 6b temp:2b DNC
  //Status bits - 00:normal, 01: stale data
  //%RH = 100*RH/(2^14-1)
  //Temp = 165*Temp/(2^14-1)-40
  //40ms measurement cycle

  //beginTransmission(address), endTransmission()
  //requestFrom(address, numBytes), read()
  data[0] = 0xA0;
  data[1] = 0x00;
  data[2] = 0x00;
  
  Wire.beginTransmission(address);
  Wire.write(data,3);
  Wire.endTransmission();
 // delay(40);
  
  data[0] = 0x1C;
  data[1] = 0x00;
  data[2] = 0x00;
  
  Wire.beginTransmission(address);
  Wire.write(data,3);
  Wire.endTransmission();
 // delay(40);
  
  Wire.requestFrom(address, (uint8_t)3);
  int i = 0;
  while (Wire.available()){
    data[i] = Wire.read();
    i++;
  }
Serial.println(data[0],BIN);
  data[0] = 0x5C;
  data[2] = 0x29; 
   
  Wire.beginTransmission(address);
  Wire.write(data,3);
  Wire.endTransmission();
 // delay(40);

//  signalBits = data[0] >>6;
//  if(signalBits > 0){
//    delay(10);
//    Serial.print("continue");
//    return;
//  }
//  
//delay(5000);
}
