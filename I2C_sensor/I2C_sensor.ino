#include <Wire.h>
uint8_t address;
uint8_t data[4];
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
  Wire.begin(address);
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
  Wire.beginTransmission(address);
  Wire.endTransmission();
  delay(40);
  Wire.requestFrom(address, (uint8_t)4);
  for(int i=0; i<4; i++){
    data[i] = Wire.read();
  }
  
  signalBits = data[0] >>6;
  if(signalBits > 0){
    delay(10);
    Serial.print("continue");
    return;
  }
  rawRH = data[0] &= 0x3F;
  rawRH = (rawRH<<8)+data[1];
  rawTemp = data[2];
  rawTemp = (rawTemp<<6)+(data[3]>>2);
  humidity = (100.0*rawRH)/(0x3FFF-1);
  temp = (165.0*rawTemp)/(0x3FFF-1)-40;
  
  Serial.print("Reading:");
  Serial.print(humidity, 5);
  Serial.print("% RH, ");
  temp = ((temp*9)/5.0)+32;
  Serial.print(temp, 5);
  //Serial.println(" degrees C");  
  Serial.println(" degrees F");
  
  delay(5000);
}
