#include <nRF24L01.h>                 
#include <RF24.h>					 //header file for NRF module
#include <RF24_config.h>             //header file including configuration settings of RF24 module
#include <SPI.h>					 //header file for communication of arduino with other modules
#include <TESPI.h>					 //header file for communication with ESP module

String data1="",oldData1="";		 //string variables for collecting data from NRF
String data2="",oldData2="";		 //sensor data split seperately and stored into 3 strings
String data3="",oldData3="";
String ifsafe="";					 //flag variable for indicating emergency 
int msg[1];							 //integer array for NRF transmission
RF24 radio(9,10);					// Arduino pins in NRF receiver

const uint64_t pipe = 0xA8A8F0F0E1LL; //pipe for message reception in NRF module
String theMessage = "";			    //string with all sensor data received
TESPI esp(Serial);

void setup(void)					//sketch starts
{
  Serial.begin(9600);
  esp.setWifiStationConfig("ICFOSS-Workshop","guest@lbs%",1);//initialise network settings on ESP module
  esp.loadWifiStationConfig();     //load configuration settings on ESP

  radio.begin();				  //begin NRF reception
  radio.openReadingPipe(1,pipe);  //initiate pipe to read to receive sensor data
  data1="101";
  data2="202";					 //dummy sensor values
  data3="303";
  radio.startListening();		//start reading from pipe
}

void loop(void)
{
  if (radio.available())
  {
      bool done = false;  
      done = radio.read(msg, 1); //read each character from pipe and store to string
      char theChar = msg[0];
      if (msg[0] != 2)
      {
          theMessage.concat(theChar);  //string contain all sensor data
      }

      else 
      {
          int fcomma = theMessage.indexOf(",");
          data1=theMessage.substring(0,fcomma);
          int fcomma2 = theMessage.indexOf(",", fcomma+1); //split single array sensor data
          data2=theMessage.substring(fcomma+1,fcomma2);    //into 3 seperate arrays
          data3=theMessage.substring(fcomma2+1);
          ifsafe = "0";									   //initialise safety flag
      }
  }
  
  String data = "/sc/update.php?p=p_101&s="+data1+"&t="+data2+"&u="+data3+"&i=" + ifsafe;
  String host_url = "smartnodes.in";                    //set host name
  if (data1 != oldData1 || data2 != oldData2 || data3 != oldData3) 
  {
     esp.HTTP_GET(data,host_url,80);  //post sensor data to cloud
  }
  
  oldData1= data1;
  oldData2= data2;				
  oldData3= data3;
  
  delay(3000);
}




