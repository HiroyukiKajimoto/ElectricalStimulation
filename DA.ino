/* DA output example*/
/* 2018 Nov. 19 Hiroyuki Kajimoto*/
#include <SPI.h>
// VSPI
#define SCLK 18 
#define MOSI 23
#define MISO 19
#define DA_CS 22
#define AD_CS 5
#define LEDPIN 12

short spiData,DA;
char channel = 0;
unsigned long time;
float freq = 1000.0;

/* DA output by AD5452(SPI)*/
void AD5452_DAout(short DA) {
    digitalWrite(DA_CS,LOW);         //enable clock
    SPI.transfer16(DA<<2);
    digitalWrite(DA_CS,HIGH);         //disable clock and load data
}

void setup() {
  pinMode(LEDPIN,OUTPUT);
  pinMode(DA_CS,OUTPUT);
  pinMode(AD_CS,OUTPUT);
  Serial.begin(921600);
  SPI.begin(SCLK, MISO, MOSI, DA_CS);
  SPI.setFrequency(48000000); 
  SPI.setDataMode(SPI_MODE0);
  SPI.setHwCs(true);
}

void loop() {
  int t;
  short DA;

  t=micros();
  if((t/1000000)%2==0){
    digitalWrite(LEDPIN, HIGH);   // sets the LED on
  }else{
    digitalWrite(LEDPIN, LOW);    // sets the LED off    
  }
  DA = (short)(2047.99 * (sin(2.0*M_PI*100*(double)t*0.000001)+1.0)); //100Hz sin wave 0.0 (0V) - 4095.0 (2.3V)
  AD5452_DAout(DA);
}

