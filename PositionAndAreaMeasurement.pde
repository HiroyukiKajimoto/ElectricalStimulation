/**************************
For Transparent TouchPanel
2016 Aug. 29-
Hiroyuki Kajimoto
***************************/

import processing.serial.*;

// The serial port:
Serial myPort;       

//user definitions
String COM_PORT="COM36"; //change this!

//hardware definitions
//   -7 -5 -3 -1  1  3  5  7
//7  64 63 62 61  1  2  3  4
//5  60 59 58 57  5  6  7  8
//3  56 55 54 53  9 10 11 12
//1  52 51 50 49 13 14 15 16
//-1 48 44 40 36 29 25 21 17
//-3 47 43 39 35 30 26 22 18
//-5 46 42 38 34 31 27 23 19
//-7 45 41 37 33 32 28 24 20

final int ELECTRODE_NUM=61;
final float[] Electrode_Pos_X ={
        0.5,  0.25,  0.0,  -0.25,  -0.5,
      0.625,  0.375,  0.125,  -0.125,  -0.375, -0.625,  
    0.75,  0.5,  0.25,  0.0,  -0.25,  -0.5,  -0.75,  
  0.875,  0.625,  0.375,  0.125,  -0.125,  -0.375,  -0.625,  -0.875,  
1.0,  0.75,  0.5,  0.25,  0.0,  -0.25,  -0.5,  -0.75,  -1.0,  
  0.875,  0.625,  0.375,  0.125,  -0.125,  -0.375, -0.625,  -0.875,  
    0.75,  0.5,  0.25,  0.0,  -0.25,  -0.5,  -0.75,  
      0.625,  0.375,  0.125,  -0.125,  -0.375,  -0.625,  
        0.5,  0.25,  0.0,  -0.25,  -0.5};

final float[] Electrode_Pos_Y ={
        0.8660254 , 0.8660254 , 0.8660254 , 0.8660254 , 0.8660254 , 
      0.649519 , 0.649519 , 0.649519 , 0.649519 , 0.649519 , 0.649519 , 
    0.4330127 , 0.4330127 , 0.4330127 , 0.4330127 , 0.4330127 , 0.4330127 , 0.4330127 , 
  0.21650635 , 0.21650635 , 0.21650635 , 0.21650635 , 0.21650635 , 0.21650635 , 0.21650635 , 0.21650635 , 
0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 0.0 , 
  -0.21650635 , -0.21650635 , -0.21650635 , -0.21650635 , -0.21650635 , -0.21650635 , -0.21650635 , -0.21650635 , 
    -0.4330127 , -0.4330127 , -0.4330127 , -0.4330127 , -0.4330127 , -0.4330127 , -0.4330127 , 
      -0.649519 , -0.649519 , -0.649519 , -0.649519 , -0.649519 , -0.649519 , 
        -0.8660254 , -0.8660254 , -0.8660254 , -0.8660254 , -0.8660254};
//software definitions
final int PC_MBED_STIM_PATTERN=0xFF;
final int PC_MBED_MEASURE_REQUEST=0xFE;
final int MBED_PC_RECEIVE_FINISHED = 0xFE;
final int MBED_PC_MEASURE_RESULT=0xFF;

// thresholds
int AREASIZE_THRESHOLD=5;
int IMPEDANCE_THRESHOLD=30;
int IMPEDANCE_INITIAL_VAL = 211;

//graphical attributes
final int WINDOW_SIZE_X=400;
final int WINDOW_SIZE_Y=400;
final int ELECTRODE_SIZE=25;

float x, y, theta=0;
color statuscolor;
int timer=0, FirstTime=0;
int[] stimulation = new int[ELECTRODE_NUM];
int[] impedance = new int[ELECTRODE_NUM];
int[] impedance_offset =  new int[ELECTRODE_NUM];
int[] impedance_diff = new int[ELECTRODE_NUM];
int  AreaSize;
float CoGX, CoGY, Sum;
int Volume = 0;


boolean SerialDataSendRequest=false;
boolean StimDataSending=false;
boolean FirstMeasurement = true;

// Impedance Distribution Measurement Function
void MeasureImpedanceDistribution() {
  int pin,rcv;

   if(myPort.available()<ELECTRODE_NUM+1){
       myPort.write((byte)PC_MBED_MEASURE_REQUEST); 
  }else { // the data is renewed.
    for (pin=0; pin<ELECTRODE_NUM; pin++) {
      impedance[pin]=myPort.read();
      impedance_diff[pin] = impedance_offset[pin] - impedance[pin];
    }

    if(FirstMeasurement == true){
      FirstMeasurement = false;
      for (pin=0; pin<ELECTRODE_NUM; pin++) {
        impedance_offset[pin] = impedance[pin];
      }
    }
      rcv = myPort.read();
  }
}

//send stimulation signal to mbed
 void SendStimulationSignal()
{
  int pin;

  myPort.write((byte)PC_MBED_STIM_PATTERN); 
  for (pin=0; pin<ELECTRODE_NUM; pin++) {
    myPort.write((byte)stimulation[pin]);
  }
}

void settings() {
   size(WINDOW_SIZE_X,WINDOW_SIZE_Y);   
}

void setup() {
  int pin;
  
  // Open the port. baud rate=921600
  myPort = new Serial(this, COM_PORT, 921600);
  //For mac users
  //myPort = new Serial(this, "/dev/tty.usbmodem1412", 921600);
  myPort.clear();
  frameRate( 60 );
  println("Now volume is set to 0. Press UP and DOWN keys to adjust volume");

}

//assume 60fps reflesh rate
void draw() { 
  int i, pin;

  //clear screen
  background(255);

  //present rotating point
  x=0.7*cos(theta);
  y=0.7*sin(theta);
  theta+=0.1;
//  theta+=0.01;
  for (pin=0; pin<ELECTRODE_NUM; pin++) {
    if((x-Electrode_Pos_X[pin])*(x-Electrode_Pos_X[pin])+(y-Electrode_Pos_Y[pin])*(y-Electrode_Pos_Y[pin])<0.07){

      stimulation[pin]=Volume; //Pulse Height. 200 →800/1024*5= 3.91mA
    } else {
      stimulation[pin]=0;
    }
  }
    SendStimulationSignal();

  //Measure impedance and calculate center of gravity and contact area (every 5 times)
  timer=(timer+1)%2;
  if (timer==0) {
    AreaSize=0;
    CoGX=0;
    CoGY=0;
    Sum=0;
    MeasureImpedanceDistribution();
    
    for (pin=0; pin<ELECTRODE_NUM; pin++) {
      if (impedance_diff[pin]>0) {
        AreaSize++;
        CoGX += Electrode_Pos_X[pin]*(float)impedance_diff[pin];
        CoGY += Electrode_Pos_Y[pin]*(float)impedance_diff[pin];
        Sum += (float)impedance_diff[pin];
      }
    }
    if (AreaSize>AREASIZE_THRESHOLD) {
      CoGX /= Sum;
      CoGY /= Sum;
    } else {
      CoGX=0;
      CoGY=0;
    }
  }

  //draw electrodes. Red: Stimulation, Green: Impedance (touch)
  for (pin=0; pin<ELECTRODE_NUM; pin++) {
    statuscolor=color(stimulation[pin], abs(impedance_diff[pin])*8, 100);
    fill(statuscolor);
    ellipse((Electrode_Pos_X[pin]*0.4+0.5)*WINDOW_SIZE_X,(-Electrode_Pos_Y[pin]*0.4+0.5)*WINDOW_SIZE_Y,ELECTRODE_SIZE,ELECTRODE_SIZE); 

  }
  //Draw CoG and contact area size
  statuscolor=color(255, 255, 0);
  fill(statuscolor, 200);
  ellipse((CoGX*0.6+0.5)*WINDOW_SIZE_X, (0.5-CoGY*0.6)*WINDOW_SIZE_Y, AreaSize, AreaSize);

}

void keyPressed() {
if (key == CODED) {
    if (keyCode == UP) {
      Volume = Volume + 1;
    } else if (keyCode == DOWN) {
      Volume = Volume -1;
    } 
  }
  if(Volume > 200) Volume = 200;
  if(Volume <0) Volume = 0;
  println("Volume is set to: ",Volume);
}