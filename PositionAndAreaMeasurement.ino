/******************************************************************************/
/* Simpel Stimulation example to be used with Processing program              */
/* Functionality: 61 electrodes stimulation and 61 electrodes measurement.    */
/* 2018 Nov. 25 Hiroyuki Kajimoto                                             */
/******************************************************************************/

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
#define ELECTRODE_NUM 61
#define HV507_NUM 1
#define PC_ESP32_STIM_PATTERN 0xFF
#define PC_ESP32_MEASURE_REQUEST 0xFE
#define ESP32_PC_RECEIVE_FINISHED 0xFE
#define ESP32_PC_MEASURE_RESULT 0xFF
#define CURRENT_FOR_MEASUREMENT 1024 //2.5mA 4096=10mA

unsigned char stim_pattern[ELECTRODE_NUM] = { 0 };
unsigned char impedance[ELECTRODE_NUM] = { 0 };
float stim[ELECTRODE_NUM] = { 0 };
bool DA_TEST = true;


/******************************************************************************/
/*  hv507FastScan                                                             */
/*  MAKE SURE THAT YOU INITIALIZE THIS FUNCTION BY FIRST CALLING              */
/*  hv507FastScan(0);                                                         */
/*  See manual for detail of this function.                                   */
/******************************************************************************/
void hv507FastScan(int usWhichPin) {
  int ii, pin;
  static int pos;

  //Load S/R
  digitalWrite(HV507_BL, HIGH);
  digitalWrite(HV507_LE, LOW);

  if (usWhichPin == 0) {
    digitalWrite(HV507_DIOB, HIGH);
    digitalWrite(HV507_CLK, HIGH);
    digitalWrite(HV507_CLK, LOW);
    pos = 0;
  } else {
    digitalWrite(HV507_DIOB, LOW);
    pin = usWhichPin - pos;
    for (ii = 0; ii < pin; ii++) {
      digitalWrite(HV507_CLK, HIGH);
      digitalWrite(HV507_CLK, LOW);
    }
    pos = usWhichPin;
  }
}

/******************************************************************************/
/*  HV507 Clear                                                                */
/******************************************************************************/
void hv507Clear(int hv507_num) {
  hv507FastScan(HV507_PIN_NUM * hv507_num);
}

/******************************************************************************/
/*  HV507 init                                                                */
/******************************************************************************/
void hv507Init(int hv507_num) {
  int pin;
  int hv507_total = HV507_PIN_NUM * hv507_num;

  digitalWrite(HV507_POL, HIGH);
  digitalWrite(HV507_BL, LOW);
  digitalWrite(HV507_LE, LOW);
  digitalWrite(HV507_CLK, LOW);


  digitalWrite(HV507_BL, HIGH);
  digitalWrite(HV507_DIOB, LOW);
  for (pin = 0; pin < hv507_total; pin++) {
    digitalWrite(HV507_CLK, HIGH);
    digitalWrite(HV507_CLK, LOW);
  }


  digitalWrite(HV507_LE, HIGH);
  digitalWrite(HV507_LE, LOW);
  digitalWrite(HV507_BL, LOW);
}


/******************************************************************************/
/* DA output by AD5452 and AD input by AD7276(SPI)                            */
/******************************************************************************/

short DAAD(short DA) {
  short AD;
  digitalWrite(DA_CS, LOW);     //enable clock
  digitalWrite(AD_CS, LOW);     //enable clock
  AD = SPI.transfer16(DA << 2);
  digitalWrite(DA_CS, HIGH);    //disable clock and load data
  digitalWrite(AD_CS, HIGH);
  return AD >> 2;               //bottom 2bits are unnecessary
}

//Measurement
void measure()
{
  int pin, i;
  short PulseHeight, AD;

  PulseHeight = CURRENT_FOR_MEASUREMENT;
  AD = DAAD(PulseHeight);
  //25us charge
  AD = DAAD(PulseHeight); //0-5mA. Simultaneous DA and AD. 2.0us
  delayMicroseconds(25);
  for (pin = 0; pin < ELECTRODE_NUM; pin++) {
    hv507FastScan(pin);
    digitalWrite(HV507_BL, HIGH);
    digitalWrite(HV507_LE, HIGH);
    digitalWrite(HV507_LE, LOW);
    AD = DAAD(PulseHeight);
    delayMicroseconds(20);
    impedance[pin] = AD >> 2;
    if (impedance[pin] > 254)impedance[pin] = 254;
    digitalWrite(HV507_BL, LOW);
  }
  DAAD(0);
  hv507Clear(HV507_NUM);    //cleaning
  digitalWrite(HV507_LE, HIGH);
  digitalWrite(HV507_LE, LOW);
  digitalWrite(HV507_BL, LOW);
}


void stimulate()
{
  int pin, i;
  short PulseHeight, AD;
  static float vol = 0.0;

  //Stimulation
  hv507FastScan(0);
  for (pin = 0; pin < ELECTRODE_NUM; pin++) {
    if (stim_pattern[pin] != 0) {
      PulseHeight = stim_pattern[pin] << 4; // max 4095 (10mA) 
      if (pin != 0) {
        hv507FastScan(pin);
      }
      digitalWrite(HV507_BL, HIGH);
      digitalWrite(HV507_LE, HIGH);
      digitalWrite(HV507_LE, LOW);
      //Cathodic Stimulation
      //digitalWrite(HV507_POL, LOW);
      //Anodic Stimulation
      digitalWrite(HV507_POL, HIGH);
      //50us stimulation
      AD = DAAD(PulseHeight); //0-5mA. Simultaneous DA and AD. 2.0us
      delayMicroseconds(50);
      AD = DAAD(0);
      digitalWrite(HV507_BL, LOW);
      //150us rest
      AD = DAAD(0);
      delayMicroseconds(150);
    }
  }
  hv507Clear(HV507_NUM);    //cleaning
  digitalWrite(HV507_LE, HIGH);
  digitalWrite(HV507_LE, LOW);
  digitalWrite(HV507_BL, LOW);
}


void setup() {
  pinMode(LEDPIN, OUTPUT);
  pinMode(DA_CS, OUTPUT);
  pinMode(AD_CS, OUTPUT);
  pinMode(HV507_DIOB, OUTPUT);
  pinMode(HV507_BL, OUTPUT);
  pinMode(HV507_POL, OUTPUT);
  pinMode(HV507_CLK, OUTPUT);
  pinMode(HV507_LE, OUTPUT);
  Serial.begin(921600);
//  SPI.begin(SCLK, MISO, MOSI, DA_CS);
  SPI.begin(SCLK, MISO, MOSI);
  SPI.setFrequency(48000000);
  SPI.setDataMode(SPI_MODE0);
  SPI.setHwCs(true);
  hv507Init(HV507_NUM); //initialize one HV507
  Serial.begin(921600);
}

void loop() {
  int t, pin;
  char rcv;

  t = micros();
  // sinusoidal wave with 1024 amplitude (2.5mA).The amplitude should be 2.5mA * 66kOhm = 165V
  if (DA_TEST == true) {
    if ((t / 1000000) % 2 == 0) {
      digitalWrite(LEDPIN, HIGH);   // sets the LED on
    } else {
      digitalWrite(LEDPIN, LOW);    // sets the LED off
    }
    DAAD((short)(512.0 * (1.0 + sin(2.0 * 3.1415926 * 100.0 * (double)t * 0.000001))));
  }
  if (Serial.available() > 0) {
    if ((t / 300000) % 2 == 0) {
      digitalWrite(LEDPIN, HIGH);   // sets the LED on
    } else {
      digitalWrite(LEDPIN, LOW);    // sets the LED off
    }
    rcv = Serial.read();
    if (rcv == PC_ESP32_STIM_PATTERN) {//if PC requires stimulation
      DA_TEST = false;
      while(Serial.available()<ELECTRODE_NUM); //wait until serial buffer is filled with data.
      for (pin = 0; pin < ELECTRODE_NUM; pin++) {//read data
        stim_pattern[pin] = Serial.read();       
        if (stim_pattern[pin] > 200)stim_pattern[pin] = 0; //sometimes, header is in the data
      }
      stimulate();
    } else if (rcv == PC_ESP32_MEASURE_REQUEST) {
      measure();
      for (pin = 0; pin < ELECTRODE_NUM; pin++) {
        Serial.write(impedance[pin]);
      }
      Serial.write(ESP32_PC_MEASURE_RESULT);
    }
  }
}

