/******************************************************/
//       THIS IS A GENERATED FILE - DO NOT EDIT       //
/******************************************************/

#include "Particle.h"
#line 1 "c:/Users/Gladys/Documents/Repos/Particles/LoRa_Test2/Server/src/Server.ino"
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

// Singleton instance of the radio driver
void updateSender();
void setup();
void loop();
void TrySendQue();
void TryGetMessages();
#line 25 "c:/Users/Gladys/Documents/Repos/Particles/LoRa_Test2/Server/src/Server.ino"
RH_RF95 rf95;
String WebHookName = "OwlNode_System01"; // Webhook name set within particle console.
//RH_RF95 rf95(5, 2); // Rocket Scream Mini Ultra Pro with the RFM95W
//RH_RF95 rf95(8, 3); // Adafruit Feather M0 with RFM95

// Need this on Arduino Zero with SerialUSB port (eg RocketScream Mini Ultra Pro)
//#define Serial SerialUSB
const int queLength = 100;
String messageQue[queLength];
int iSending = 0;
int iReceiving = 0;
long long lastSent;
bool readyToSend;
bool haveSentall = false;
int led = D7;


Timer timer(1000, updateSender);

void updateSender(){
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
  Serial.begin(9600);
  //while (!Serial)
  //  ; // Wait for serial port to be available
  delay(1000);
  if (!rf95.init())
    Serial.println("init failed");
  else 
  {
    rf95.setFrequency(915);
    rf95.setTxPower(23,false);
  }
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

  
  Log.info("Max lora message length is: "+ String(rf95.maxMessageLength()));
  Log.info("Finished Setup loop.");
  Log.info("************************");
  lastSent = Time.now();
}

void loop()
{
  TryGetMessages();
  // Try Send from que
  TrySendQue();
  // Try get messages.
  TryGetMessages();
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
    }
    
  }else{
    if (iReceiving != 0)
    {
      haveSentall = false;
     // Serial.println("\nSENT ALL\n");
    }
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

      int msgStart = newMsg.indexOf("{");
      int msgEnd = newMsg.indexOf("}");
      if (msgStart > 0 && msgEnd > 0)
      {
        String sender = newMsg.substring(0, msgStart);
        Serial.println(sender);
        String msgRcived = newMsg.substring(msgStart, msgEnd + 1);
        Serial.println(msgRcived);

        if (sender.length() > 0 && msgRcived.length() > 0)
        {
          msgRcived.replace("Dv", "Dd");
          msgRcived.remove(msgRcived.length() - 1);
          msgRcived.concat(",\"Si\": \"" + String(rf95.lastRssi())+"\" ");
          msgRcived.concat(",\"Dv\": \"Server01\" }");
          messageQue[iReceiving] = String(msgRcived);
          iReceiving++;
          if (iReceiving >= queLength)
          {
            iReceiving = 0;
          }
          // Send a reply

          sender.concat(' ');
          char dataMsg[sender.length()+1];
          sender.toCharArray(dataMsg, sender.length());

          uint8_t data[sender.length()+1];
          for (size_t i = 0; i < sender.length()+1; i++)
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
