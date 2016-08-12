void setup() {
  // put your setup code here, to run once:
  DDRD = 0xFF;
  DDRB = 0xFF;
  PORTD = 0x00;
  PORTB = 0x00;

  PORTD = 0x34;
  delay(10);
  pulseEnable;
  delay(10);
  pulseEnable;
  delay(10);
  pulseEnable;
  PORTD = 0x0C;
  delay(10);
  pulseEnable();
  delay(10);
  pulseEnable();
  delay(10);
  pulseEnable();
  PORTD = 0x01;
  delay(1);
  pulseEnable();
  delay(1);
  pulseEnable();
  delay(1);
  pulseEnable();
  
  PORTB |= 0x01;
  digitalWrite(11,HIGH);
}

void loop() {
  // put your main code here, to run repeatedly:

  byte a[] = "string";

  printString(a);
  
  delay(100);
  PORTB |=0x10;
  delay(5000);
  digitalWrite(12,LOW);

}
void printString(byte text[]){
byte current = text[0];
int count = 0;
  while (current != 0){
    PORTD = current;
    pulseEnable();
    current = text[++count];
  }  
}

void pulseEnable(void) {
  PORTB &= 0xFB;
  delayMicroseconds(1);    
  PORTB |= 0x04;
  delayMicroseconds(1);    // enable pulse must be >450ns
  PORTB &= 0xFB;
  delayMicroseconds(100);   // commands need > 37us to settle
}
