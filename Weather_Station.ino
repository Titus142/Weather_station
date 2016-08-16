#include "Adafruit_GFX.h"
#include "Adafruit_HX8357.h"
#include "Adafruit_STMPE610.h"
#include <string.h>
//#include <SD.h>
#include <Process.h>
#include <FileIO.h>
#include <SPI.h>
#include <Wire.h>


// These are 'flexible' lines that can be changed
#define SD_CS 4
#define TFT_CS 12
#define TFT_DC 8
#define TFT_RST -1 // RST can be set to -1 if you tie it to Arduino's reset

#define TS_MINY 110//130 //80 //110
#define TS_MINX 230//150 //110 //270
#define TS_MAXY 3800//4000 //940 //3800
#define TS_MAXX 170//3800 //900  //170


//////////////////////////////////////////////////

// Color definitions
#define BGCOLOR  0x0358


/////////////////////////////////////////////////

String logo = "http://arduino.cc/asciilogo.txt";
String url = "http://wserv.ddns.net/weather.txt";
//String fcastDay = "/mnt/sd/arduino/www/fcastday.txt";
//String fcastNight = "/mnt/sd/arduino/www/fcastnight.txt";
//String fcastTom = "/mnt/sd/arduino/www/fcasttom.txt";
char data[50];
char fCast[150];
const unsigned long interval = 600000; //interval set for 10 min to pull data for gauges for a total of 288 API request / day
const unsigned long BLinterval = 30000; //backlight timeout
unsigned long currentMillis;
unsigned long TFTcurrentMillis;
unsigned long previousMillis = 0;
unsigned long touchMillis = 0;
int blState = 1;
uint16_t x, y;
uint8_t z;

typedef struct //struct for weather data
{
  int temp;
  int hum;
  int dew;
  int pop;
  int bar;
} WDATA ;

WDATA wdata;

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

/******************
    ... SETUP...
******************/

void setup() {
  Serial.begin(9600);
  Bridge.begin();
  Console.begin();
  FileSystem.begin();
  tft.begin(HX8357D);
  touch.begin();

  pinMode(7, OUTPUT);
  digitalWrite(7, HIGH);

  //Startup Splashscreen
  tft.fillScreen(HX8357_BLACK);
  tft.setCursor(0, 0);
  tft.setRotation(3);
  tft.setTextColor(HX8357_WHITE);
  tft.setTextSize(1);
  runCurl(logo, 1);
  delay(3000);

  drawGui();



} //End of Setup

/*****************
    ... LOOP...
*****************/

void loop() {

  currentMillis = millis();

  //if (touch.touched())
  if(!touch.bufferEmpty())
  {
    TS_Point p = touch.getPoint();
    while (touch.touched())
    {
      p = touch.getPoint();
    }
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 320);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 480);

    if (p.x > 275)
    {
      if (p.y < 55)
      {
        getFcast(1);
        drawGui();
        tft.drawRoundRect(2, 275, 45, 40, 5, HX8357_WHITE);
        tft.setTextSize(4);
        tft.print(fCast);
      }
      else if (p.y < 210)
      {
        getFcast(2);
        drawGui();
        tft.drawRoundRect(154, 275, 45, 40, 5, HX8357_WHITE);
        tft.setTextSize(4);
        tft.print(fCast);
      }
      else if (p.y < 360)
      {
        getFcast(3);
        drawGui();
        tft.drawRoundRect(305, 275, 45, 40, 5, HX8357_WHITE);
        tft.setTextSize(4);
        tft.print(fCast);
      }
    }
  }
  if ((unsigned long)(currentMillis - previousMillis) >= interval)
  {
    runCurl(url, 2);
    parseData();
    writeData();
    previousMillis = millis();
  }

} //END OF LOOP

/**********************
    ... Run Curl ...
**********************/

void runCurl(String url, int i) {

  Process p;        // Create a process and call it "p"
  p.begin("curl");  // Process that launch the "curl" command
  p.addParameter(url); // Add the URL parameter to "curl"
  p.run();      // Run the process and wait for its termination

  if (i == 1) //get data for startup splash, shows internet connection is working
  {
    while (p.available() > 0)
    {
      char c = p.read();
      tft.print(c);
    }
  }
  else if (i == 2) //get data for gauge output
  {
    int n = 0;
    while (p.available() > 0)
    {
      char c = p.read();
      data[n] = c;
      n++;
      data[n] = '\0';
    }
  }
} //END OF CURL


/*************************
    ... Parse Wdata ...
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
    ... Write to gauge ...
****************************/

void writeData()
{
  wdata.temp = map(wdata.temp, -5, 105, 0, 255);
  wdata.hum = map(wdata.hum, 0, 100, 0, 255);
  wdata.pop = map(wdata.pop, 0, 100, 0, 255);
  wdata.bar = map(wdata.bar, 960, 1070, 0, 255);
  wdata.dew = map(wdata.dew, -10, 100, 0, 255);
  analogWrite(tempPin, wdata.temp);
  analogWrite(humPin, wdata.hum);
  analogWrite(popPin, wdata.pop);
  analogWrite(barPin, wdata.bar);
  analogWrite(dewPin, wdata.dew);
}

void drawGui()
{
  tft.setTextSize(2);
  tft.fillScreen(HX8357_BLACK);

  //Day forecast button 0, 55, 275, 320
  tft.fillRoundRect(2, 275, 45, 40, 5, HX8357_BLUE);
  tft.setCursor(52, 280);
  tft.print("Daily");
  tft.setCursor(52, 297);
  tft.print("Forecast");

  //Night Forecast button
  tft.fillRoundRect(154, 275, 45, 40, 5, HX8357_BLUE);
  tft.setCursor(206, 280);
  tft.print("Nightly");
  tft.setCursor(206, 297);
  tft.print("Forecast");

  //tomorrow forecast
  tft.fillRoundRect(305, 275, 45, 40, 5, HX8357_BLUE);
  tft.setCursor(357, 280);
  tft.print("Tomorrow's");
  tft.setCursor(357, 297);
  tft.print("Forecast");

  tft.setCursor(0, 0);
}

/*********************************
   ... Get Forecast from txt ...
*********************************/

void getFcast(int n)
{
  if (n == 1)
  {
    File f = FileSystem.open("/mnt/sd/arduino/www/fcastday.txt", FILE_READ);
    int i = 0;
    while (f.available())
    {
      char c = f.read();
      fCast[i] = c;
      i++;
      fCast[i] = '\0';
    }
    f.close();
  }
  else if (n == 2)
  {
    File f = FileSystem.open("/mnt/sd/arduino/www/fcastnight.txt", FILE_READ);
    int i = 0;
    while (f.available())
    {
      char c = f.read();
      fCast[i] = c;
      i++;
      fCast[i] = '\0';
    }
    f.close(); 
  }
  else if (n == 3)
  {
    File f = FileSystem.open("/mnt/sd/arduino/www/fcasttom.txt", FILE_READ);
    int i = 0;
    while (f.available())
    {
      char c = f.read();
      fCast[i] = c;
      i++;
      fCast[i] = '\0';
    }
    f.close();
  }
}
