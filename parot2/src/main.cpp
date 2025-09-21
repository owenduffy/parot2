
#define VERSION "2.01"
#include <Arduino.h>
#include <EEPROM.h>

//#if defined(ARDUINO_AVR_ATtiny412)
#if defined(ARDUINO_attinyxy2)
//pins
//UPDI PA0 (5) 6
//TXD PA6 (0) 2
//RXD PA7 (1) 3 - not used
#define BUTTON 2 //PA1 4
#define PWR 3 //PA2 5
#define PTT 4 //PA3 7
#define LED 1 //PA7 3
//HardwareSerial &MySerial=Serial;
#endif //ARDUINO_AVR_ATtiny412
#if defined(ARDUINO_attinyxy4)
//pins
//UPDI PA0 (11) 10 
//TXD PB2 (5) 7
//RXD PB3 (4) 6 - not used
#define BUTTON 2 //PA6 4
#define PWR 3 //PA7 5
#define PTT 6 //PB1 8
#define LED 1 //PA5 3
#endif //ARDUINO_attinyxy4

enum statet{rst,off,heat,on,cool};
statet state=rst;
enum buttont{n,s,l};
unsigned long ts=0,te;
struct{
  char ever=0;
  uint16_t opt;
  char sl;
  uint16_t hdt;
  uint16_t tot;
  uint16_t cdt;
  uint16_t rct;
  signed char ckadj; //unused
} parms;
//========================================================================
buttont button(){
  uint16_t state=0;
  unsigned long t0,t1;
  t0=millis();
  while(1){
    if(state==0 && digitalRead(BUTTON))return n;
    if(state==0){
      t1=millis()-t0;
      if(t1>parms.sl)state=1;
    }
    if(state==1){
      while(!digitalRead(BUTTON));
        t1=millis()-t0;
        if(t1<10*parms.sl)return s;
        return l;
    }
  }
}
//========================================================================
void setup(){
  int ever;

  ever=EEPROM.read(0);
  if(ever!=1){
    while(1);
  }
  EEPROM.get(0,parms);
  pinMode(BUTTON,INPUT_PULLUP);
  digitalWrite(PWR,HIGH);
  digitalWrite(PTT,HIGH);
  digitalWrite(LED,HIGH);
  pinMode(LED,OUTPUT);
  pinMode(PWR,OUTPUT);
  pinMode(PTT,OUTPUT);
}
//========================================================================
void loop() {
  switch(state){
    case rst:
    digitalWrite(LED,HIGH);
    digitalWrite(PWR,HIGH);
    digitalWrite(PTT,HIGH);
    delay(parms.rct*1000);
    while(!digitalRead(BUTTON));
    state=off;
    break;
    case off:
      switch(button()){
        case n:
          break;
        case s:
          state=heat;
          ts=millis();
          break;
        case l:
          state=rst;
          break;
      }
      break;
    case heat:
      te=millis()-ts;
      digitalWrite(LED,(te/100)&0x1);
      digitalWrite(PWR,LOW);
      switch(button()){
        case n:
          break;
        case s:
          state=on;
          ts=millis();
          break;
        case l:
          state=rst;
          break;
      }
      if(te>1000*parms.hdt){
        state=on;
        ts=millis();
        break;
      }
      break;
    case on:
      te=millis()-ts;
      digitalWrite(PTT,LOW);
      digitalWrite(LED,LOW);
      if(te>1000*parms.tot){
        state=rst;
      }
      switch(button()){
        case n:
          break;
        case s:
          state=cool;
          ts=millis();
          break;
        case l:
          state=rst;
          break;
      }
      break;
    case cool:
      te=millis()-ts;
      digitalWrite(LED,(te/500)&0x1);
      digitalWrite(PTT,HIGH);
      if(te>1000*parms.cdt){
        state=rst;
        break;
      }
      switch(button()){
        case n:
          break;
        case s:
          state=on;
          ts=millis();
          break;
        case l:
          state=rst;
          break;
      }
      break;
  }
}
