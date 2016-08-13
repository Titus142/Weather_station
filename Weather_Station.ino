#include <SPI.h>
#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include <Wire.h>
#include "Adafruit_STMPE610.h"
#include <Process.h>
#include <string.h>

// These are 'flexible' lines that can be changed
#define SD_CS 4
#define TFT_CS 12
#define TFT_DC 8
#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset

// Color definitions
#define BLACK    0x0000
#define BLUE     0x001F
#define RED      0xF800
#define GREEN    0x07E0
#define CYAN     0x07FF
#define MAGENTA  0xF81F
#define YELLOW   0xFFE0
#define WHITE    0xFFFF

/////////////////////////////////////////////////

String logo = "http://arduino.cc/asciilogo.txt";
String url = "http://wserv.ddns.net/weather.txt";
char data[300];
const unsigned long interval = 600000; //interval set for 10 min to pull data for gauges for a total of 288 API request / day
unsigned long previousMillis = 0;

int counter = 0;

typedef struct WDATA //struct for weather data
{
  int temp;
  int hum;
  int dew;
  int pop;
  int bar;
};

WDATA wdata; 

const String destServer = "192.168.1.189";

////////////////////////////////////////////
/*...  Pin variables ...*/


const int tempPin = 11;
const int humPin = 10;
const int popPin = 9;
const int barPin = 6;
const int dewPin = 5;


////////////////////////////////////////////

Adafruit_HX8357 tft = Adafruit_HX8357(TFT_CS, TFT_DC, TFT_RST);
Adafruit_STMPE610 touch = Adafruit_STMPE610();

void setup() {
  Serial.begin(9600);
  Bridge.begin();
  tft.begin(HX8357D);
  touch.begin();

tft.fillScreen(BLACK);
tft.setCursor(0, 0);
tft.setRotation(3);
tft.setTextColor(WHITE);
tft.setTextSize(1);
runCurl(logo, 1);
delay(3000);
tft.setTextSize(4);
tft.fillScreen(BLACK);
tft.setCursor(0, 0);
  


} //End of Setup

           /*****************
           *   ... LOOP...  *
           *****************/

void loop() {


unsigned long currentMillis = millis();
Serial.println(currentMillis);
if ((unsigned long)(currentMillis - previousMillis) >= interval)
{
  Serial.println("IF Started here ~~~~~~");
  runCurl(url, 2);
  parseData();
  tft.fillScreen(BLACK);
  tft.setCursor(0, 0);
  tft.println(wdata.temp);
  tft.println(wdata.hum);
  tft.println(wdata.dew);
  tft.println(wdata.pop);
  tft.println(wdata.bar);  

  writeData();
  tft.println(counter);
  counter++;
  previousMillis = millis();
}

} //END OF LOOP

           /**********************
           *   ... Run Curl ...  *
           **********************/

void runCurl(String url, int i) {

  Process p;        // Create a process and call it "p"
  p.begin("curl");  // Process that launch the "curl" command
  p.addParameter(url); // Add the URL parameter to "curl"
  p.run();      // Run the process and wait for its termination

  if (i == 1)
  {
    while (p.available()>0) 
    {
      char c = p.read();
      tft.print(c);
    }
  }
  else if (i == 2)
  {
    int n = 0;
    while (p.available()>0) 
    {
      char c = p.read();
      data[n] = c;
      n++;
      data[n] = '\0';
    }
  }
} 

           /*************************
           *   ... Parse Wdata ...  *
           *************************/

void parseData()
{

   char dataT[sizeof(data) + 1];
   strcpy(dataT, data);
   char *temp = strtok(dataT, ",");
   char *hum = strtok(NULL, ",");
   char *dew = strtok(NULL, ",");
   char *pop = strtok(NULL, ",");
   char *bar = strtok(NULL, "\0");
  

   wdata.temp = atoi(temp);
   wdata.hum = atoi(hum);
   wdata.pop = atoi(pop);
   wdata.bar = atoi(bar);
   wdata.dew = atoi(dew);
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
  wdata.dew = map(wdata.dew, 50, 100, 0, 255);
  analogWrite(tempPin, wdata.temp);
  analogWrite(humPin, wdata.hum);
  analogWrite(popPin, wdata.pop);
  analogWrite(barPin, wdata.bar);
  analogWrite(dewPin, wdata.dew);
}
