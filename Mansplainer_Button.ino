/*
 * Creation & Computation - Digital Futures, OCAD University
 * Kate Hartman / Nick Puckett
 * 
 * publish 1 random value and 1 potentiometer value when a button is pressed
 *  
 * 
 * 
 */
#include <ArduinoJson.h> 
#include <SPI.h>

#include <WiFi101.h>
#define PubNub_BASE_CLIENT WiFiClient
#include <PubNub.h>

static char ssid[] = "ocadu-embedded";      //SSID of the wireless network
static char pass[] = "internetofthings";    //password of that network
int status = WL_IDLE_STATUS;                // the Wifi radio's status

const static char pubkey[] = "pub-c-fc8a1e2a-74e9-401f-9bbe-4e58d7e9547d";  //get this from your PUbNub account
const static char subkey[] = "sub-c-156b0cde-c88d-11e7-9695-d62da049879f";  //get this from your PubNub account

const static char pubChannel[] = "Kristy"; //She is only publishing and not subscribing so only 1 channel 


unsigned long lastRefresh = 0;
int publishRate = 2000;

//int sensorPin1 = A0;                  

int buttonPrev;
int buttonVal; //button value


int myVal1;                       //variables to hold values to send
int myVal2;


int buttonPin = 10;  // the pin the + leg of the button is attached to
int buttonState;  //variable that will hold the On / Off state of the button.


void setup() 
{

  pinMode(buttonPin, INPUT_PULLUP); //input_pullup so it doesnt require a resistor
  Serial.begin(9600);
  connectToServer();


}

void loop() 
{
  //buttonVal = digitalRead(buttonPin);

 buttonState = digitalRead(buttonPin);  //read the value on the pin and store it in the variable

 Serial.print("Button State is: ");     //print the value to the Serial Monitor
  
  
  //myVal1 = analogRead(sensorPin1);
  myVal2 = digitalRead(buttonPin); //to send button data to myVal2




//if((buttonVal==1)&&(buttonPrev==0))  //trigger the feed update with a button, uses both current and prev value to only change on the switch
{  
  Serial.println(buttonState);
publishToPubNub();
}

buttonPrev = buttonVal; //store the value of this cycle to compare next loop

}

// from Nick's example to connect to wifi and pubnub
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

// from Nick's example
void publishToPubNub()
{
  WiFiClient *client;
  StaticJsonBuffer<800> messageBuffer;                    //create a memory buffer to hold a JSON Object
  JsonObject& pMessage = messageBuffer.createObject();    //create a new JSON object in that buffer
  
 ///the imporant bit where you feed in values
  
  //Message["State1"] = myVal1;                      //add a new property and give it a value
  pMessage["State2"] = myVal2;                     //add a new property and give it a value


///                                                       //you can add/remove parameter as you like
  
  //pMessage.prettyPrintTo(Serial);   //uncomment this to see the messages in the serial monitor
  
  
  int mSize = pMessage.measureLength()+1;                     //determine the size of the JSON Message
  char msg[mSize];                                            //create a char array to hold the message 
  pMessage.printTo(msg,mSize);                               //convert the JSON object into simple text (needed for the PN Arduino client)
  
  client = PubNub.publish(pubChannel, msg);                      //publish the message to PubNub

  if (!client)                                                //error check the connection
  {
    Serial.println("client error");
    delay(1000);
    return;
  }
  
  if (PubNub.get_last_http_status_code_class() != PubNub::http_scc_success)  //check that it worked
  {
    Serial.print("Got HTTP status code error from PubNub, class: ");
    Serial.print(PubNub.get_last_http_status_code_class(), DEC);
  }
  
  while (client->available()) 
  {
    Serial.write(client->read());
  }
  client->stop();
  Serial.println("Successful Publish");
  
}

