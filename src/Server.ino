
#include "RFM69-Particle.h"
#include "RFM69_ATC.h"
#include "RFM69registers.h"
#include <SPI.h>

// Adjustments to library to work with Particle Photon including in Web IDE by Jurie Pieterse
// Forked library for Photon at https://github.com/bloukingfisher/RFM69/
// Serial NOT required to confirm working - you can watch your Particle Console logs!

/* RFM69 library and code by Felix Rusu - felix@lowpowerlab.com
// Get libraries at: https://github.com/LowPowerLab/
// Make sure you adjust the settings in the configuration section below !!!
// **********************************************************************************
// Copyright Felix Rusu, LowPowerLab.com
// Library and code by Felix Rusu - felix@lowpowerlab.com
// **********************************************************************************
// License
// **********************************************************************************
// This program is free software; you can redistribute it 
// and/or modify it under the terms of the GNU General    
// Public License as published by the Free Software       
// Foundation; either version 3 of the License, or        
// (at your option) any later version.                    
//                                                        
// This program is distributed in the hope that it will   
// be useful, but WITHOUT ANY WARRANTY; without even the  
// implied warranty of MERCHANTABILITY or FITNESS FOR A   
// PARTICULAR PURPOSE. See the GNU General Public        
// License for more details.                              
//                                                        
// You should have received a copy of the GNU General    
// Public License along with this program.
// If not, see <http://www.gnu.org/licenses></http:>.
//                                                        
// Licence can be viewed at                               
// http://www.gnu.org/licenses/gpl-3.0.txt
//
// Please maintain this license information along with authorship
// and copyright notices in any redistribution of this code
// **********************************************************************************/


//*********************************************************************************************
// *********** IMPORTANT SETTINGS - YOU MUST CHANGE/CONFIGURE TO FIT YOUR HARDWARE *************
//*********************************************************************************************
int16_t NETWORKID = 100;  //the same on all nodes that talk to each other
int16_t NODEID = 1;  

//Match frequency to the hardware version of the radio on your Feather
//#define FREQUENCY     RF69_433MHZ
//#define FREQUENCY     RF69_868MHZ
#define FREQUENCY      RF69_915MHZ
#define ENCRYPTKEY     null //"sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HCW    true // set to 'true' if you are using an RFM69HCW module

//*********************************************************************************************

#define RFM69_CS      A2 
#define RFM69_IRQ     2
#define RFM69_IRQN    2 //On Photon it is the same unlike Arduino
#define RFM69_RST     6

int16_t packetnum = 0;  // packet counter, we increment per xmission

RFM69 radio = RFM69(RFM69_CS, RFM69_IRQ, IS_RFM69HCW, RFM69_IRQN);  //initialize radio with potential custom pin outs; otherwise you may use for default: RFM69 radio;

//*********************************************************************************************
// ***********                  Wiring the RFM69 Radio to Photon                  *************
//*********************************************************************************************
/* Arduino wiring provided for reference, color 
    Photon      Arduino	    RFM69	Color
    GND         GND	        GND	    Black
    3V3         3.3V	    VCC	    Red
    A2          10	        NSS	    Yellow
    A3          13	        SCK	    Green
    A5          11	        MOSI	Blue
    A4          12	        MISO	Violet
    D2          2	        DI00	Gray
                            ANA	    Antenna
    D6                      RST     Optional
*/

void setup() 
{
 
    Serial.begin(115200); // Open serial monitor at 115200 baud to see ping results.
    Particle.publish("RFM69 RX Startup setup","Completed",360,PRIVATE);
    Particle.publish("WiFi signal",String(WiFi.RSSI()),360,PRIVATE);
    Serial.println("RFM69 Based Receiver");
  
    // Hard Reset the RFM module - Optional
  pinMode(RFM69_RST, OUTPUT);
  digitalWrite(RFM69_RST, HIGH);
  delay(100);
  digitalWrite(RFM69_RST, LOW);
  delay(100);
  
    // Initialize radio
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
  if (IS_RFM69HCW) {
    radio.setHighPower();    // Only for RFM69HCW & HW!
  }
  
  // To improve distance set a lower bit rate. Most libraries use 55.55 kbps as default
  // See https://lowpowerlab.com/forum/moteino/rfm69hw-bit-rate-settings/msg1979/#msg1979
  // Here we will set it to 9.6 kbps instead 
  radio.writeReg(0x03,0x0D); //set bit rate to 9k6
  radio.writeReg(0x04,0x05);
  
  radio.setPowerLevel(10); // power output ranges from 0 (5dBm) to 31 (20dBm)
                          // Note at 20dBm the radio sources up to 130 mA! 
                         // Selecting a power level between 10 and 15 will use ~30-44 mA which is generally more compatible with Photon power sources
                        // As reference, power level of 10 transmits successfully at least 300 feet with 0% packet loss right through a home, sufficient for most use
    
  radio.encrypt(ENCRYPTKEY);
  
  Serial.print("\nListening at ");
  Serial.print(FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(" MHz");
}


//=========================MAIN LOOP===========================================
void loop() {

Serial.print("."); //THis gives us a neat visual indication of time between messages received

  //check if something was received (could be an interrupt from the radio)
  if (radio.receiveDone())
  {
    //print message received to serial
    Serial.println(" ");
    Serial.print('[');Serial.print(radio.SENDERID);Serial.print("] ");
    Serial.print((char*)radio.DATA);
    Serial.print("   [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    //send message to Particle console if not using serial
    String RXMessage = "[" + String(radio.SENDERID) + "]  " + String((char*)radio.DATA) + " [RSSI: " + String(radio.RSSI) + "]";
    Particle.publish("Message received",RXMessage,360,PRIVATE);

    //check if received message contains Hello
    if (strstr((char *)radio.DATA, "Hello"))
    {
      //check if sender wanted an ACK
      if (radio.ACKRequested())
      {
        radio.sendACK();
        Serial.println(" - ACK sent");
      }
      else {Serial.println(" - No ACK sent");}
    }  
  }

  radio.receiveDone(); //put radio in RX mode
  
  delay(1000);


} //end loop

 


/*


// rf95_server.pde
// -*- mode: C++ -*-
// Example sketch showing how to create a simple messageing server
// with the RH_RF95 class. RH_RF95 class does not provide for addressing or
// reliability, so you should only use RH_RF95  if you do not need the higher
// level messaging abilities.
// It is designed to work with the other example rf95_client
// Tested with Anarduino MiniWirelessLoRa, Rocket Scream Mini Ultra Pro with
// the RFM95W, Adafruit Feather M0 with RFM95

//  Connections:
//  Photon RFM96
//  D2----------->DIO0
//  A2----------->NSS
//  A3----------->SCK
//  A4----------->MISO
//  A5----------->MOSI
//  GND--------->GND
//  3V3---------->3.3V IN

#include <SPI.h>
#include <RF95/RH_RF95.h>
//#include "SerialBufferRK.h"

// Singleton instance of the radio driver
RH_RF95 rf95;
String WebHookName = "OwlNode_System01"; // Webhook name set within particle console.
//String ImgHookName = "Img_Upload_01";
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB
const int queLength = 200;
String messageQue[queLength];
int iSending = 0;
int iReceiving = 0;
long long lastSent; // seconds
bool readyToSend;
bool haveSentall = false;
int led = D7;



bool inDebugMode = false;

int turnOnPin = D0;
int activePin = D1;

float minForPhoto = 10;
long long lastPhoto;

int nSignals = 0;
String message = "No signal received";


//const int imgSize = 30720;
//const int imgMessageSize = 1;
//uint8_t lastImage[imgSize];
//char imgMessage[imgMessageSize];
//uint32_t imgIndex = 0;
//bool haveImage = false;
//bool sentPartImage = false;
int partCount = 0;
int imageCount = 0;

//SerialBuffer<1024> serBuf(Serial1);
int totalReceived = 0;

Timer timer(1000, updateSender);

void updateSender()
{
  //if(readyToSend)
  //Serial.println("Ready to send heartbeat");
  readyToSend = true;
}

void setup()
{
  // Rocket Scream Mini Ultra Pro with the RFM95W only:
  // Ensure serial flash is not interfering with radio communication on SPI bus
  //  pinMode(4, OUTPUT);
  //  digitalWrite(4, HIGH);

  pinMode(led, OUTPUT);
  pinMode(activePin, INPUT);
  Serial1.begin(1000000, SERIAL_8N1);
  //serBuf.setup();

  Serial.begin(9600);
  //while (!Serial)
  //  ; // Wait for serial port to be available
  delay(1000);
  if (!rf95.init())
    Serial.println("init failed");
  else
  {
    rf95.setFrequency(915);
    rf95.setTxPower(23, false);
  }
  
  //Particle.variable("messagecount", &nSignals, INT);
  //if (Particle.variable("message", message, STRING) == false)
  //{
      // variable not registered!
  //}
  timer.start();
  // Defaults after init are 434.0MHz, 13dBm, Bw = 125 kHz, Cr = 4/5, Sf = 128chips/symbol, CRC on

  // The default transmitter power is 13dBm, using PA_BOOST.
  // If you are using RFM95/96/97/98 modules which uses the PA_BOOST transmitter pin, then
  // you can set transmitter powers from 5 to 23 dBm:
  //  driver.setTxPower(23, false);
  // If you are using Modtronix inAir4 or inAir9,or any other module which uses the
  // transmitter RFO pins and not the PA_BOOST pins
  // then you can configure the power transmitter power for -1 to 14 dBm and with useRFO true.
  // Failure to do that will result in extremely low transmit powers.
  //  driver.setTxPower(14, true);

  Log.info("Max lora message length is: " + String(rf95.maxMessageLength()));
  Log.info("Finished Setup loop.");
  Log.info("************************");
  lastSent = Time.now();
  lastPhoto = Time.now();
}

void loop()
{
  if(inDebugMode)
  {
TryPrintMessags();
  }
  else{
  //TryGetPhoto();
  TryGetMessages();
  // Try Send from que
  TrySendQue();
  //TrySendImage();
  // Try get messages.
  TryGetMessages();
  }
}





void TrySendQue()
{
  if (iSending != iReceiving && readyToSend)
  {
    Particle.publish(WebHookName, messageQue[iSending], 60, PUBLIC); // WebHook to Google Sheets
    Serial.println("Sent to cloud: ");
    Serial.println(messageQue[iSending]);
    Serial.println(" ");
    lastSent = Time.now();
    iSending++;
    if (iSending >= queLength)
    {
      iSending = 0;
    }
    readyToSend = false;
  }

  if (!haveSentall)
  {
    if (iSending == 0 && iReceiving == 0)
    {
      haveSentall = true;
      Serial.println("\nSENT ALL\n");
      nSignals = 0;

    }
  }
  else
  {
    if (iReceiving != 0)
    {
      haveSentall = false;
      // Serial.println("\nSENT ALL\n");
    }
  }
}

void TryPrintMessags(){
  
  if (rf95.available())
  {

    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      //      RH_RF95::printBuffer("request: ", buf, len);
      Serial.println("got request: ");
      Serial.println((char *)buf);
      String newMsg = String((char *)buf);
      Serial.println(newMsg);
    }
    else
    {
      Serial.println("recv failed");
    }
    Serial.println("");
  }
}

void TryGetMessages()
{
  if (rf95.available())
  {

    // Should be a message for us now
    uint8_t buf[RH_RF95_MAX_MESSAGE_LEN];
    uint8_t len = sizeof(buf);
    if (rf95.recv(buf, &len))
    {
      digitalWrite(led, HIGH);
      //      RH_RF95::printBuffer("request: ", buf, len);
      Serial.println("got request: ");
      Serial.println((char *)buf);
      String newMsg = String((char *)buf);
      //Serial.println(newMsg);

      //message = String((char *)buf);
      nSignals+=1;

      int msgStart = newMsg.indexOf("{");
      int msgEnd = newMsg.indexOf("}");
      if (msgStart > 0 && msgEnd > 0)
      {
        String sender = newMsg.substring(0, msgStart);
        //Serial.println(sender);
        String msgRcived = newMsg.substring(msgStart, msgEnd + 1);
        //Serial.println(msgRcived);

        if (sender.length() > 0 && msgRcived.length() > 0)
        {
          msgRcived.replace("Dv", "Dd");
          msgRcived.remove(msgRcived.length() - 1);
          msgRcived.concat(",\"Si\": \"" + String(rf95.lastRssi()) + "\" ");

          float dh = getMessage(msgRcived,"Dh").toFloat();
          int zone = getMessage(msgRcived,"Zo").toInt();
          int year = getMessage(msgRcived,"Yr").toInt();
          
          time_t sensorTime = getTimeFromDecimal(year,zone, dh);

          msgRcived.concat(",\"Mo\": \"" + String(Time.month(sensorTime)) + "\" ");
          msgRcived.concat(",\"Da\": \"" + String(Time.day(sensorTime)) + "\" ");
          msgRcived.concat(",\"Hr\": \"" + String(Time.hour(sensorTime)) + "\" ");
          msgRcived.concat(",\"Mi\": \"" + String(Time.minute(sensorTime)) + "\" ");
          msgRcived.concat(",\"Sc\": \"" + String(Time.second(sensorTime)) + "\" ");


          msgRcived.concat(",\"Dv\": \"Server01\" }");
          messageQue[iReceiving] = String(msgRcived);
          iReceiving++;
          if (iReceiving >= queLength)
          {
            iReceiving = 0;
          }
          // Send a reply

          sender.concat(' ');
          char dataMsg[sender.length() + 1];
          sender.toCharArray(dataMsg, sender.length());

          uint8_t data[sender.length() + 1];
          for (size_t i = 0; i < sender.length() + 1; i++)
          {
            data[i] = uint8_t(dataMsg[i]);
          }

          rf95.send(data, sizeof(data));
          rf95.waitPacketSent();

          Serial.print("Sent -");
          Serial.print((char *)data);
          Serial.println("- as a reply.");
        }
        else
        {
          Serial.println("ID or message not found. No reply message sent.");
        }
      }
      else
      {
        Serial.println("No start or end bracket found, no reply message sent.");
      }
      digitalWrite(led, LOW);
    }
    else
    {
      Serial.println("recv failed");
    }
    Serial.println("");
  }
}

*/


String getMessage(String msgRcived, String item){

  int dh_start = msgRcived.indexOf(item);
  int dh_Fin = msgRcived.indexOf(",",dh_start);
  String dh_string = msgRcived.substring(dh_start+4,dh_Fin);
  Serial.println(msgRcived.substring(dh_start+4,dh_Fin));
  return dh_string;
  
}

time_t getTimeFromDecimal(float Dh_time)
{
  intmax_t secEpoc_Now = Time.now();
  float DhYr_Now = getDecimalHr(secEpoc_Now);
  intmax_t secYr_Now = DhYr_Now * 3600;
  intmax_t secYr_time = Dh_time * 3600;
  intmax_t secEPOC_time = secEpoc_Now - secYr_Now + secYr_time;
  time_t timeOut = secEPOC_time;

  return timeOut;
}

time_t getTimeFromDecimal(int year , int zone, float Dh_time)
{
  int additionalSeconds = 0;
  if (year > 2020)
  {
    additionalSeconds = floor((year - 2021) / 4 + 1) * 86400;
  }
  long secStartOfYear = 1546300800 + (year - 2019) * 31536000 + additionalSeconds;
  long secDhtime = Dh_time * 3600 + zone * 3600;
  //long diffFromSoY = secFromEpoc - secStartOfYear;

  intmax_t secEpoc_Now = Time.now();
  float DhYr_Now = getDecimalHr(secEpoc_Now);
  intmax_t secYr_Now = DhYr_Now * 3600;
  intmax_t secYr_time = Dh_time * 3600;
  //intmax_t secEPOC_time = secEpoc_Now - secYr_Now + secYr_time;

  intmax_t secEPOC_time = secDhtime + secStartOfYear;
  time_t timeOut = secEPOC_time;

  return timeOut;
}

float getDecimalHr()
{
  //Log.info("Getting decimal hours for now.");
  int year = Time.year();
  long secFromEpoc = Time.now();
  int additionalSeconds = 0;
  if (year > 2020)
  {
    additionalSeconds = floor((year - 2021) / 4 + 1) * 86400;
  }
  long secStartOfYear = 1546300800 + (year - 2019) * 31536000 + additionalSeconds;
  long diffFromSoY = secFromEpoc - secStartOfYear;
  float hrs = diffFromSoY * 1.0 / 3600.0;

  return hrs;
}


float getDecimalHr(time_t timeIn)
{
  //Log.info("Calculating Dh from %d seconds", (intmax_t)time);
  int year = Time.year(timeIn);
  long secFromEpoc = (intmax_t)timeIn;
  int additionalSeconds = 0;
  if (year > 2020)
  {
    additionalSeconds = floor((year - 2021) / 4 + 1) * 86400;
  }
  long secStartOfYear = 1546300800 + (year - 2019) * 31536000 + additionalSeconds;
  //Log.info("Seconds for the start of year: " + String(secStartOfYear));
  long diffFromSoY = secFromEpoc - secStartOfYear;
  float hrs = diffFromSoY * 1.0 / 3600.0;
  return hrs;
}




