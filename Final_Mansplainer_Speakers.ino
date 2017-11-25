/*
 * Mansplainer Speakers
 * Kristy Boyce / Tommy Ting
 * 
 * Creation and Computation DIGF-6037-001
 * wifi connected speakers to be triggered to play music by a wifi connected button 
 *  
 * 
 * 
 */
 
#include <ArduinoJson.h> 
#include <SPI.h>


#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

// include SPI, MP3 and SD libraries
#include <SPI.h>
#include <SD.h>
#include <Adafruit_VS1053.h>

// These are the pins used
#define VS1053_RESET   -1     // VS1053 reset pin (not used!)

//Feather M0 or 32u4
#if defined(__AVR__) || defined(ARDUINO_SAMD_FEATHER_M0)
  #define VS1053_CS       6     // VS1053 chip select pin (output)
  #define VS1053_DCS     10     // VS1053 Data/command select pin (output)
  #define CARDCS          5     // Card chip select pin
  // DREQ should be an Int pin *if possible* (not possible on 32u4)
  #define VS1053_DREQ     9     // VS1053 Data request, ideally an Interrupt pin

#endif

Adafruit_VS1053_FilePlayer musicPlayer = 
Adafruit_VS1053_FilePlayer(VS1053_RESET, VS1053_CS, VS1053_DCS, VS1053_DREQ, CARDCS);
  
static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-fc8a1e2a-74e9-401f-9bbe-4e58d7e9547d";  //get this from your PUbNub account
const static char subkey[] = "sub-c-156b0cde-c88d-11e7-9695-d62da049879f";  //get this from your PubNub account

const static char pubChannel[] = "Tommy"; //choose a name for the channel to publish messages to
const static char subChannel[] = "Kristy"; //choose a name for the channel to publish messages to

unsigned long lastRefresh = 0;
int publishRate = 2000;

int sensorPin1 = A0; 

int State2;

int yourVal1 = 1; //we had to make this to = 1 as when this value was always at a 0 to start with, so when the button presses down to generate 0 it wouldn't read a change in values and therefore the music wouldn't start

void setup() {
  
Serial.begin(115200); //this number was from the example of the music maker feather wing library
  

// if you're using Bluefruit or LoRa/RFM Feather, disable the BLE interface
//pinMode(8, INPUT_PULLUP);544

// Wait for serial port to be opened, remove this line for 'standalone' operation
//  while (!Serial) { delay(1); }

// example code from Music Maker Feather Wing Library
  Serial.println("\n\nAdafruit VS1053 Feather Test");
  
  if (! musicPlayer.begin()) { // initialise the music player
     Serial.println(F("Couldn't find VS1053, do you have the right pins defined?"));
     while (1);
  }

 //Serial.println(F("VS1053 found"));
 
 // musicPlayer.sineTest(0x44, 500);    // Make a tone to indicate VS1053 is working Tommy - this was annoying as it generated a loud beep so we cancelled it out
  
  if (!SD.begin(CARDCS)) {
  Serial.println(F("SD failed, or not present"));
    while (1);  // don't do anything more
  }
  Serial.println("SD OK!");
  
  // list files
  printDirectory(SD.open("/"), 0);
  
  // Set volume for left, right channels. lower numbers == louder volume!
  musicPlayer.setVolume(10,10);
  
#if defined(__AVR_ATmega32U4__) 
  // Timer interrupts are not suggested, better to use DREQ interrupt!
  // but we don't have them on the 32u4 feather...
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_TIMER0_INT); // timer int
#elif defined(ESP32)
  // no IRQ! doesn't work yet :/
#else
  // If DREQ is on an interrupt pin we can do background
  // audio playing
  musicPlayer.useInterrupt(VS1053_FILEPLAYER_PIN_INT);  // DREQ int
#endif
  
  // to run the void that connects to wifi and pubnub 
  connectToServer();
}


void loop() 
{
  
  if (musicPlayer.stopped()) {
    Serial.println("Done playing music");
//    while (1) {
//      delay(10);  // we're done! do nothing...
//    }
  }

  Serial.println(yourVal1);
  if (yourVal1 == 0) {
    
   Serial.print(".");
  // to play track001 and track002 if yourVal1 = 0 
  Serial.println(F("Playing full track 001"));
  musicPlayer.playFullFile("track001.mp3");

  Serial.println(F("Playing track 002"));
  musicPlayer.startPlayingFile("track002.mp3");

  delay(100);
}

  readFromPubNub();
   
  Serial.print("State2 ");
  Serial.println(yourVal1);
 
  lastRefresh=millis();   

//from Nick's examples to connect to wifi and pubnub
void connectToServer()
{
  WiFi.setPins(8,7,4,2); //This is specific to the feather M0
 
  status = WiFi.begin(ssid, pass);                    //attempt to connect to the network
  Serial.println("***Connecting to WiFi Network***");


 for(int trys = 1; trys<=10; trys++)                    //use a loop to attempt the connection more than once
 { 
    if ( status == WL_CONNECTED)                        //check to see if the connection was successful
    {
      Serial.print("Connected to ");
      Serial.println(ssid);
  
      PubNub.begin(pubkey, subkey);                      //connect to the PubNub Servers
      Serial.println("PubNub Connected"); 
      break;                                             //exit the connection loop     
    } 
    else 
    {
      Serial.print("Could Not Connect - Attempt:");
      Serial.println(trys);

    }

    if(trys==10)
    {
      Serial.println("I don't this this is going to work");
    }
    delay(1000);
 }

  
}

// example from Music Maker Feather Wing Library
void printDirectory(File dir, int numTabs) {
   while(true) {
     
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');
     }
     Serial.print(entry.name());
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

// from Nick's example
void readFromPubNub()
{
  StaticJsonBuffer<1200> inBuffer;                    //create a memory buffer to hold a JSON Object
  WiFiClient *sClient =PubNub.history(subChannel,1);

  if (!sClient) {
    Serial.println("message read error");
    delay(1000);
    return;
  }

  while (sClient->connected()) 
  {
    while (sClient->connected() && !sClient->available()) ; // wait
    char c = sClient->read();
    JsonObject& sMessage = inBuffer.parse(*sClient);
    
    if(sMessage.success())
    {
      //sMessage.prettyPrintTo(Serial); //uncomment to see the JSON message in the serial monitor
      yourVal1 = sMessage["State2"];  //
      Serial.print("State2");
      Serial.println(yourVal1);
      //yourVal2 = sMessage["randoVal2"];
      //Serial.print("randoVal2 ");
      //Serial.println(yourVal2);
      
    }
    
    
  }
  
  sClient->stop();

}



