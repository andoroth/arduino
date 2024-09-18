#include <Wire.h>
#include <Servo.h>

#define CLOCK 4
#define DATA 3
#define LOAD 2
#define SERVO 9
#define FAN 10
#define EXTID 0x27
#define INTID 0x29
#define CYCLEMAX 120
#define I_DECAY 0.99
#define I_WEIGHT 30
#define P_WEIGHT 128
#define D_WEIGHT 64
#define SERVO_OPEN 65
#define SERVO_CLOSED 130

Servo myServo;
bool coolingOn = false, internalDec = false, externalDec = false;
int cycleCount = 0, fanLevel = 0;
double setpoint = 65;
double setRead,d0=0,i1=0,p1=0,d1=0;
double externalTemp = 0, internalTemp = 0, internalTemp0 = 75.0;
uint8_t externalDisp = 0, internalDisp = 0;


void setup() {
  // put your setup code here, to run once:
  DDRD = 0xFC;
  DDRB = 0xFF;
  PORTD = 0x00;
  PORTB = 0x00;
  Wire.begin();
  Serial.begin(9600);
  Wire.setClock(100000);
  myServo.attach(SERVO);
}

void loop() {
  externalTemp = readTemp(EXTID);
  setDisplayTemp(&externalDec, &externalDisp, externalTemp);
  setRead = analogRead(A0) / 1024.0;
  setRead = 60 + 15.0 * setRead;
  if (setRead - setpoint < -0.6 | setRead - setpoint > 0.6) {
    int count = 0;
    while (count < 10) {
      internalDisp = round(setRead);
      updateDisplay();
//      int servoLevel = round(90*(analogRead(A0) / 1024.0))+65;
//      Serial.println(servoLevel,DEC);
//      myServo.write(servoLevel);
      setpoint = setRead;
      i1 = 0;
      setRead = 60 + 15.0 * analogRead(A0) / 1024.0;
      count++;
      if (round(setRead) != round(setpoint)) {
        count = 0;
      }
      wait(20000);
    }
  }
  internalTemp = readTemp(INTID);
  //  internalTemp = 80;
  setDisplayTemp(&internalDec, &internalDisp, internalTemp);
  checkActivationState(); //Write controller code here
  updateDisplay();
  Serial.print("Ext: ");
  Serial.print(externalTemp);
  Serial.print(" Int: ");
  Serial.print(internalTemp);
  Serial.print(" Fan Level: ");
  Serial.println(fanLevel);
  Serial.print("I: ");
  Serial.print(i1);
  Serial.print(" P: ");
  Serial.print(p1);
  Serial.print(" D: ");
  Serial.println(d1);
  wait(20000);
}
void setDisplayTemp(bool* dec, uint8_t* disp, double temp) {
  if (temp > 99) {
    *disp = 99;
    *dec = false;
  } else if (temp >= 10) {
    *disp = floor(temp);
    *dec = false;
  } else if (temp >= 0) {
    *disp = floor(10 * temp);
    *dec = true;
  } else {
    *disp = 0;
    *dec = true;
  }
}
void updateDisplay(void) {
  byte outByte = dec2dispByte(internalDisp % 10);
  writeDigit(outByte | coolingOn);
  outByte = dec2dispByte(internalDisp / 10);
  writeDigit(outByte | internalDec);
  outByte = dec2dispByte(externalDisp % 10);
  writeDigit(outByte | 0x01);//Divider is always set high
  outByte = dec2dispByte(externalDisp / 10);
  writeDigit(outByte | externalDec);
  digitalWrite(LOAD, HIGH);
  digitalWrite(LOAD, LOW);
}
byte dec2dispByte(byte inDec) {
  switch (inDec) {
    case 0:
      return B11111100;
    case 1:
      return B01100000;
    case 2:
      return B11011010;
    case 3:
      return B11110010;
    case 4:
      return B01100110;
    case 5:
      return B10110110;
    case 6:
      return B10111110;
    case 7:
      return B11100000;
    case 8:
      return B11111110;
    case 9:
      return B11110110;
    default:
      return 0x00;
  }
}
float readTemp(byte address) {
  uint8_t data[4];
  uint8_t signalBits;
  uint16_t rawTemp, rawRH;
  double temp, humidity;

  Wire.beginTransmission(address);
  Wire.endTransmission();
  delay(40);
  Wire.requestFrom(address, (uint8_t)4);
  for (int i = 0; i < 4; i++) {
    data[i] = Wire.read();
    //Serial.println(data[i],BIN);
  }
  signalBits = data[0] >> 6;
  if (signalBits > 0) {
    delay(10);
    Serial.println("continue");
    return 3.3;
  }

  rawRH = data[0] &= 0x3F;
  rawRH = (rawRH << 8) + data[1];
  rawTemp = data[2];
  rawTemp = (rawTemp << 6) + (data[3] >> 2);
  humidity = (100.0 * rawRH) / (0x3FFF - 1);
  temp = 32 + ((165.0 * rawTemp) / (0x3FFF - 1) - 40) * 9.0 / 5.0;
  //delay(5000);
  return temp;
}
void checkActivationState(void) {
  if (cycleCount == 0) {
    fanLevel = round(I_WEIGHT*i1+P_WEIGHT*p1-D_WEIGHT*(d1-d0));
    d0 = d1;
    d1 = 0;
    p1 = 0;
    if (fanLevel>255) fanLevel = 255;
    coolingOn = fanLevel>0;
    if (fanLevel<150) fanLevel = 0;
    if (coolingOn) {
      analogWrite(FAN, fanLevel); 
      myServo.write(SERVO_OPEN);
    }
    else {
      analogWrite(FAN, 0);
      myServo.write(SERVO_CLOSED);
    }
  }
  i1 = ((internalTemp-setpoint)/CYCLEMAX)+i1*I_DECAY;
  p1 = p1+(internalTemp-setpoint)/CYCLEMAX;
  d1 = (internalTemp)/CYCLEMAX;

  cycleCount++;
  if (cycleCount > CYCLEMAX) {
    cycleCount = 0;
  }
}
void writeDigit(byte outChar) {
  digitalWrite(CLOCK, LOW);
  for (int i = 0; i < 8; i++) {
    if (bitRead(outChar, i) == 0) {
      digitalWrite(DATA, LOW);
    } else {
      digitalWrite(DATA, HIGH);
    }
    digitalWrite(CLOCK, HIGH);
    digitalWrite(CLOCK, LOW);
  }
}
long wait(long waitTime) {
  long tmp;
  for (int i = 0; i < waitTime; i++) {
    tmp = tmp * 2 + 30 - i;

    Serial.print("");
  }
  return tmp;
}


