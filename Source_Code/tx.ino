#include <SPI.h>                      //header file for communication of arduino with other modules
#include <nRF24L01.h>
#include <RF24.h>                     //header file for NRF module
#include <RF24_config.h>              //header file including configuration settings of RF24 module
int msg[1];                           //integer array for NRF transmission
int sens1,sens2,sens3;                //flag variables for 3 sensors to alert emergency
RF24 radio(47,49);                    // Atmega pins for NRF transmitter

const uint64_t pipe =0xA8A8F0F0E1LL ; //pipe for message transmission in NRF module
const int xpin = A1;                  //x-axis of the accelerometer
int init_reading=0;                   //initial reading of accelerometer
int i;                                //count variable for accelerometer instances 
float tempC;                          //final temp sensor value
int reading;                          //reads from analog pin of Atmega
int tempPin = 0;                      //pin on Atmega connected to temp sensor
unsigned long currentMillis;
unsigned long previousMillis = 0,pd;  //Variables for pulse oximeter
const int pulse = 2;                  //analog pin for pulse oximeter
int ps,finaltemp;                     //variables for calculation
String trans;                         //string variable collecting all sensor values for NRF transmitter

void setup()                          //sketch starts
{                        
  analogReference(INTERNAL2V56);      // initialize the serial communications for Arduino Atmega 
  Serial.begin(9600);
  Serial3.begin(9600);                //Serial3 port for GSM module
  init_reading= analogRead(xpin);     //initial accelerometer reading
  pinMode(pulse, INPUT);              //initialise mode of pin connected to oximeter
  radio.begin();                      //start NRF communication
  radio.openWritingPipe(pipe);        //Setup pipe for NRF transmission in write mode 
}

void loop(void) 
{
  delay(10000);                       // 10s delay for reading sensors
  sens1=0;
  sens2=0;                            //initiaize sensor flag variables
  sens3=0;
  acc();                              //function call for accelerometer 
  temperature();                      //function call for temperature sensor
  oxi();                              //fn call for pulse oximeter
  nrftx();                            //fn call to transmit msg by NRF module
    if(sens1==1||sens2==1||sens3==1)
     sms();                           //send emergency message if any abnormal condition occurs
}

void nrftx()                          //NRF transmission
{                        
  int messageSize = trans.length();   //string trans contains all sensor values

  for (int i = -1; i < messageSize; i++) 
  {
    int charToSend[1];
    charToSend[0] = trans.charAt(i);  //transmit character by character to NRF receiver
    radio.write(charToSend,1);        //write to pipe 
  }   
  msg[0] = 2; 
  radio.write(msg,1);
}

void acc()                            //accelerometer code
{     
  Serial.print("Position: ");
  trans=String(analogRead(xpin));     //get reading from Atmega
  Serial.println(trans);              //store sensor data in string
  if(i==5)                            //take 5 consecutive readings
  {
    if(analogRead(xpin)==init_reading)//check with threshold value
    {
      Serial.println("Emergency");  
        sens1=1;                      //set flag variable in case of emergency
    }
    i=0;
  }
  else
    i++;
}

void temperature()                    //temp sensor code
{
  reading = analogRead(tempPin);      //read sensor data
  tempC=(5*reading*100.0)/1024.0;     //apply conversion factor
  tempC/=2;                           
  Serial.print("TEMP: ");
  finaltemp=tempC;                    //convert to another variable for storing
  trans.concat(",");                  //store with previous sensor data
  trans.concat(String(finaltemp));
  Serial.println(tempC);
  if(tempC>=37||tempC<=20)            //check current data with temp range
  {  
    Serial.println(" EMERGENCY  ");
  }
}

void oxi()                            //pulse oximeter code
{ 
  Serial.println("Pulse");
  ps = digitalRead(pulse);            //read sensor data 
  if(ps==1)
  {
	  while(ps==1)
	  ps = digitalRead(pulse);
  }
  if(ps==0)                           //count delay between an up and down pulse
  {
   	while(ps==0)
	  ps = digitalRead(pulse);
	  while(ps==1)
	  ps = digitalRead(pulse);
  }
  previousMillis = millis();
	while(ps==0)
	ps = digitalRead(pulse);
  currentMillis = millis();
  pd=currentMillis-previousMillis;   //measured pulse
  trans.concat(",");
  trans.concat(String(pd));          //store with previuos sensor data
  Serial.println(pd);
  Serial.print("ALL SENSOR VALUE: ");
  Serial.println(trans);
  if(pd<=300||pd>=600)               //check pulse with threshold values
  {
    Serial.println("EMERGENCY HIGH PULSE!!");//alert if abnormal
    sens3=1;
  }
}


void sms()                           //GSM Module code
{
  Serial3.print("\r"); 
  delay(1000);
  Serial3.print("AT\r");            //initiate AT codes
  delay(1000);
  Serial3.print("AT+CMGF=1\r");     //set GSM mode to SMS code
  delay(1000);
  Serial3.print("AT+CMGS=\"8281911852\"\r"); //send SMS to this number in case of emergency
  delay(1000);
  Serial3.print("CAUTION: EMERGENCY!!\n Patient: Susan John Address:'Peace Villa',TC2/314,Rose Nagar,Kowdiar P.O,TVM");
  delay(1000);                      //contents of emergency message
  Serial3.print("\r");
  delay(1000);
  Serial3.write(0x1A);              //write to serial output
  delay(1000); 
}




