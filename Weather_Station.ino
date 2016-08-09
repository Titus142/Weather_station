#include <SparkFunESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <string.h>

#define arraySize 250 //TODO set this to minimum needed

char data[arraySize]; //array for HTTP GET input

typedef struct WDATA //struct for weather data
{
  int temp;
  int hum;
  int pop;
  int bar;
};

WDATA wdata; 
////////////////////////////////////////////
/*... Server Connection Variables ...*/

const char mySSID[] = "and meatballs";
const char myPSK[] = "secretsquirrel";
const String getStr = "GET ";
String url;
const String httpRequest = " HTTP/1.1\n" 
                           "Host: 192.168.1.189\n"
                           "Connection: close\n\n";

const char destServer[] = "192.168.1.189";
////////////////////////////////////////////
/*...  Pin variables ...*/

const int tempPin = 3;
const int humPin = 5;
const int popPin = 6;
const int barPin = 10;


           /*******************
           *   ... SETUP ...  *
           *******************/
           
void setup() {

//Pin Setup

pinMode(tempPin, OUTPUT);
pinMode(humPin, OUTPUT);
pinMode(popPin, OUTPUT);
pinMode(barPin, OUTPUT);

//Wifi conect
  Serial.begin(9600);
  
  // initializeESP8266() verifies communication with the WiFi
  // shield, and sets it up.
  initializeESP8266();

  // connectESP8266() connects to the defined WiFi network.
  connectESP8266();

  // displayConnectInfo prints the Shield's local IP
  // and the network it's connected to.
  displayConnectInfo();


}

           /******************
           *   ... LOOP ...  *
           ******************/
           
void loop() {
Serial.println();
 
 url = getUrl();

 //remove last char 
 int strl = url.length() - 1;
 url.remove(strl);
 
 Serial.println();
 Serial.println(url);
 
  getData(); //returns in char data
  
  Serial.println(F(".........."));
  Serial.println(data);
  Serial.println(F(".........."));
  
  parseData(); //returns wdata struct

  Serial.println(wdata.temp);
  Serial.println(wdata.hum);
  Serial.println(wdata.pop);
  Serial.println(wdata.bar);  

  writeData(); //write data to gauges


} // End of Loop

//~~~~~~~~~~~~~~~~~~FUNCTIONS~~~~~~~~~~~~~~~~~~~~~~//

           /**********************
           *   ... GET DATA ...  *
           **********************/
           
void getData()
{
  
  int index = 0; 
  char inChar; //incoming data buffer


  ESP8266Client client;
  
  Serial.println(F("Getting data..."));

  int retVal = client.connect(destServer, 80); //connect to server
  if (retVal <= 0)
  {
    Serial.println(F("Failed to connect to server."));
    return;
  }
  else
  {
    Serial.println(F("Connected to host."));
  }

/*.. GET request ..*/

 Serial.print(getStr + url + httpRequest);
 client.print(getStr + url + httpRequest);

  //Serial.print(getStr + url);
  //client.print(getStr + url);

 Serial.println();

  /*.. get incoming data and put in data string ..*/
  
  while (client.available())
  {
    //Serial.write(client.read()); // read() gets the FIFO char
      inChar = client.read();
      data[index] = inChar;
      index++;
      data[index] = '\0'; // Add a null at the end
  }
  
  // connected() is a boolean return value - 1 if the 
  // connection is active, 0 if it's closed.
  if (client.connected())
    
    client.stop(); // stop() closes a TCP connection.
    Serial.println();
    Serial.println(F("Disconected from Host."));
    Serial.println(F("~~~~~~~~~~~~~~~~~~~~~~"));

} //end of getData

           /********************************
           *   ... DisplayConnectInfo ...  *
           ********************************/

void displayConnectInfo()
{
  char connectedSSID[24];
  memset(connectedSSID, 0, 24);
  // esp8266.getAP() can be used to check which AP the
  // ESP8266 is connected to. It returns an error code.
  // The connected AP is returned by reference as a parameter.
  int retVal = esp8266.getAP(connectedSSID);
  if (retVal > 0)
  {
    Serial.print(F("Connected to: "));
    Serial.println(connectedSSID);
  }

  // esp8266.localIP returns an IPAddress variable with the
  // ESP8266's current local IP address.
  IPAddress myIP = esp8266.localIP();
  Serial.print(F("My IP: ")); Serial.println(myIP);
}

           /*****************************
           *   ... Initialize Wifi ...  *
           *****************************/
           
void initializeESP8266()
{
  // esp8266.begin() verifies that the ESP8266 is operational
  // and sets it up for the rest of the sketch.
  // It returns either true or false -- indicating whether
  // communication was successul or not.
  // true
  int test = esp8266.begin();
  if (test != true)
  {
    Serial.println(F("Error talking to ESP8266."));
    errorLoop(test);
  }
  Serial.println(F("ESP8266 Shield Present"));
}
  
           /*********************
           *   ... Connect ...  *
           *********************/
           
void connectESP8266()
{
  // The ESP8266 can be set to one of three modes:
  //  1 - ESP8266_MODE_STA - Station only
  //  2 - ESP8266_MODE_AP - Access point only
  //  3 - ESP8266_MODE_STAAP - Station/AP combo
  // Use esp8266.getMode() to check which mode it's in:
  int retVal = esp8266.getMode();
  if (retVal != ESP8266_MODE_STA)
  { // If it's not in station mode.
    // Use esp8266.setMode([mode]) to set it to a specified
    // mode.
    retVal = esp8266.setMode(ESP8266_MODE_STA);
    if (retVal < 0)
    {
      Serial.println(F("Error setting mode."));
      errorLoop(retVal);
    }
  }
  Serial.println(F("Mode set to station"));

  // esp8266.status() indicates the ESP8266's WiFi connect
  // status.
  // A return value of 1 indicates the device is already
  // connected. 0 indicates disconnected. (Negative values
  // equate to communication errors.)
  retVal = esp8266.status();
  if (retVal <= 0)
  {
    Serial.print(F("Connecting to "));
    Serial.println(mySSID);
    // esp8266.connect([ssid], [psk]) connects the ESP8266
    // to a network.
    // On success the connect function returns a value >0
    // On fail, the function will either return:
    //  -1: TIMEOUT - The library has a set 30s timeout
    //  -3: FAIL - Couldn't connect to network.
    retVal = esp8266.connect(mySSID, myPSK);
    if (retVal < 0)
    {
      Serial.println(F("Error connecting"));
      errorLoop(retVal);
    }
  }
} //end of connect

           /*************************
           *   ... Parse Wdata ...  *
           *************************/
void parseData()
{

   char dataT[sizeof(data) + 1];
   strcpy(dataT, data);
                strtok(dataT, "<");
   char *temp = strtok(NULL, ",");
   char *hum = strtok(NULL, ",");
   char *pop = strtok(NULL, ",");
   char *bar = strtok(NULL, ">");

   wdata.temp = atoi(temp);
   wdata.hum = atoi(hum);
   wdata.pop = atoi(pop);
   wdata.bar = atoi(bar);
}

           /****************************
           *   ... Write to gauge ...  *
           ****************************/
void writeData()
{
  wdata.temp = map(wdata.temp, -5, 105, 0, 255);
  wdata.hum = map(wdata.hum, 0, 100, 0, 255);
  wdata.pop = map(wdata.pop, 0, 100, 0, 255);
  wdata.bar = map(wdata.bar, 960, 1070, 0, 255);

  analogWrite(tempPin, wdata.temp);
  analogWrite(humPin, wdata.hum);
  analogWrite(popPin, wdata.pop);
  analogWrite(barPin, wdata.bar);
}

           /************************
           *   ... Error Loop ...  *
           ************************/
           
void errorLoop(int error)
{
  Serial.print(F("Error: ")); Serial.println(error);
  Serial.println(F("Looping forever."));
  for (;;)
    ;
}

////////////////////////////////////////////////////////////

        /*... TEMP FUNCTIONS FOR DEBUGGING ..*/
        
////////////////////////////////////////////////////////////
// serialTrigger prints a message, then waits for something
// to come in from the serial port.
void serialTrigger(String message)
{
  Serial.println();
  Serial.println(message);
  Serial.println();
  while (!Serial.available())
    ;
  while (Serial.available())
    Serial.read();
}


/////////////////////////////////////////////////////////

String getUrl()
{
  
  String urlIn;
  Serial.println(F("Enter address to GET"));
  while(!Serial.available())
  {}
  if (Serial.available())
  {
    urlIn = Serial.readString();
  }
  return urlIn;
}

////////////////////////////////////////////////////////

