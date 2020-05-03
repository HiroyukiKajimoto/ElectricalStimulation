/* HV507 output example*/
/* 2018 Nov. 20 Hiroyuki Kajimoto*/
#include <SPI.h>
#define SCLK 18 
#define MOSI 23
#define MISO 19
#define DA_CS 22
#define AD_CS 5
#define LEDPIN 12
#define HV507_DIOB 13
#define HV507_BL 25
#define HV507_POL 27
#define HV507_CLK 14
#define HV507_LE 26
#define HV507_PIN_NUM 64
#define ELECTRODE_NUM 64
#define HV507_NUM 1

/******************************************************************************/
/*  hv507FastScan                                                             */
/*  MAKE SURE THAT YOU INITIALIZE THIS FUNCTION BY FIRST CALLING              */
/*  hv507FastScan(0);                                                         */
/*  See manual for detail of this function.                                   */
/******************************************************************************/
void hv507FastScan(int usWhichPin){
    int ii,pin;
    static int pos;

    //Load S/R
    digitalWrite(HV507_BL,HIGH);   
    digitalWrite(HV507_LE,LOW);

    if(usWhichPin==0){
        digitalWrite(HV507_DIOB,HIGH);   
        digitalWrite(HV507_CLK,HIGH);   
        digitalWrite(HV507_CLK,LOW);
        pos = 0;
    }else{
        digitalWrite(HV507_DIOB,LOW);
        pin = usWhichPin-pos;
        for(ii=0;ii<pin;ii++){
            digitalWrite(HV507_CLK,HIGH);   
            digitalWrite(HV507_CLK,LOW);
        }
        pos = usWhichPin;
    }
}    

/******************************************************************************/
/*  HV507 Clear                                                                */
/******************************************************************************/
void hv507Clear(int hv507_num){
    hv507FastScan(HV507_PIN_NUM * hv507_num);
}    

/******************************************************************************/
/*  HV507 init                                                                */
/******************************************************************************/
void hv507Init(int hv507_num){
    int pin;
    int hv507_total = HV507_PIN_NUM * hv507_num;

    digitalWrite(HV507_POL,HIGH);   
    digitalWrite(HV507_BL,LOW);
    digitalWrite(HV507_LE,LOW);
    digitalWrite(HV507_CLK,LOW);
    
    
    digitalWrite(HV507_BL,HIGH);
    digitalWrite(HV507_DIOB,LOW);
    for(pin=0; pin<hv507_total; pin++){
        digitalWrite(HV507_CLK,HIGH);
        digitalWrite(HV507_CLK,LOW);
    }

   
    digitalWrite(HV507_LE,HIGH);
    digitalWrite(HV507_LE,LOW);
    digitalWrite(HV507_BL,LOW);
}


/******************************************************************************/
/* DA output by AD5452(SPI)                                                   */
/******************************************************************************/

void DAout(short DA) {
    digitalWrite(DA_CS,LOW);         //enable clock
    SPI.transfer16(DA<<2);
    digitalWrite(DA_CS,HIGH);         //disable clock and load data
}

void setup() {
  pinMode(LEDPIN,OUTPUT);
  pinMode(DA_CS,OUTPUT);
  pinMode(AD_CS,OUTPUT);
  pinMode(HV507_DIOB,OUTPUT);
  pinMode(HV507_BL,OUTPUT);
  pinMode(HV507_POL,OUTPUT);
  pinMode(HV507_CLK,OUTPUT);
  pinMode(HV507_LE,OUTPUT);
  Serial.begin(921600);
  SPI.begin(SCLK, MISO, MOSI, DA_CS);
  SPI.setFrequency(48000000); 
  SPI.setDataMode(SPI_MODE0);
  SPI.setHwCs(true);
  hv507Init(HV507_NUM); //initialize one HV507
}

void loop() {
  int t,pin;

  t=micros();
  if((t/1000000)%2==0){
    digitalWrite(LEDPIN, HIGH);   // sets the LED on
  }else{
    digitalWrite(LEDPIN, LOW);    // sets the LED off    
  }
  for (pin=0; pin<ELECTRODE_NUM; pin++) {
      hv507FastScan(pin);
      digitalWrite(HV507_BL, HIGH);
      digitalWrite(HV507_LE, HIGH);
      digitalWrite(HV507_LE,LOW);

      //50us stimulation
      DAout(2048); //1024=2.5mA, 2048=5mA, 4095=10mA
      delayMicroseconds(100);
      DAout(0);
      digitalWrite(HV507_BL,LOW);
      delayMicroseconds(200); //More than 200us rest is recommended for 10mA 50us pulse, 100us rest is 
  }
  hv507Clear(HV507_NUM);    //cleaning
  digitalWrite(HV507_LE, HIGH);
  digitalWrite(HV507_LE,LOW);
  digitalWrite(HV507_BL,LOW);
  delay(10); //wait 10ms
}

