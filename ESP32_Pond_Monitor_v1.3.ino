#include <WiFi.h>             // standard library
#include <WebServer.h>        // standard library
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFMPlus.h>  // Include TFMini Plus Library v1.5.0
#include <StopWatch.h>

#include "web_interface_1.3.h"         // .h file that stores our webpage interface
#include <HTTPClient.h>
// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// replace this with your homes intranet connect parameters
#define LOCAL_SSID "RatNet"
#define LOCAL_PASS "P1nouche"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Pond"
#define AP_PASS "Fish"

// Individual SMS event triggers
const char* eventLiquid = "https://maker.ifttt.com/trigger/Liquid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventSolid = "https://maker.ifttt.com/trigger/Solid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTemp = "https://maker.ifttt.com/trigger/Temp_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTds = "https://maker.ifttt.com/trigger/Tds_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";

// Code to access internal CPU temperature 
#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

// variables to store measure data and sensor states
int online = 0;
int offCount = 0;
float VoltsA0 = 0, VoltsA1 = 0;
bool senderOverride = false; 
bool smsSolidOverride, smsTempOverride, smsTdsOverride, smsLevelOverride, smsWarnOverride = false;       
bool LED0 = false, SomeOutput = false;
uint32_t SensorUpdate = 0;

// LIQUID LEVEL VARIABLES
int level_XML = 0;
int level_dist = 0;
int16_t tfDist = 0;                                        
int pondLevelVal = 0; 
int levelConsistFull = 0;
int levelConsistEmpty = 0;
int warnConsistFull = 0;
int warnConsistEmpty = 0;
float distance1 = 0.0;                               
String lvlString = "999";
const int numDist1Readings = 5;
int readingsDist1[numDist1Readings];
int readIndexDist1 = 0;                 
unsigned long totalDist1 = 0;                   
float avgDist1 = 0;
float duration1;   

// TEMP SENSOR VARIABLES:
float temp_XML = 0;
const int oneWireBus = 4;
byte tempSensor1[] = {0x28, 0xFF, 0xAF, 0x82, 0x0E, 0x00, 0x00, 0x3E};  // Unique DS18B20 address - need to change this for each sensor
int tempConsistHot=0;
int tempConsistCold=0;
float tempObjecC = 0.0;
float tempAmbientC = 23.7;
float maxTemp=0;
String tempString = "999";              
float pondTempVal = 0.0;
float previousTemp;

// TDS SENSOR VARIABLES:
int tds_XML = 0;
#define VREF 5.0                                  // analog reference voltage(Volt) of the ADC
#define SCOUNT  30                                // sum of sample point
int analogBuffer[SCOUNT];                         // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, tdsVal = 0;
String tdsString = "999"; 
int tdsConsistHigh = 0;
int tdsConsistLow = 0;

// SOLID LEVEL VARIABLES
uint8_t feed_buf[9] = {0};                                 // An array that holds data
int solid_XML = 0;
int solid_dist = 0;                                         // The variable "distance2" will contain the distance value in millimeter from FishFeeder distance sensor
int solidLevelVal = 0;
int solidConsistFull=0;
int solidConsistLow=0;
String solidString = "999";
int soldiLevel; 

// FILTER PRESSURE SENSOR VARIABLES:
float press_XML = 0.0;
float filterPressZero = 96.0;                      //analog reading of pressure transducer at 0psi
float filterPressMax = 139.0;                      //analog reading of pressure transducer at 100psi
float filterPresstransducermaxPSI = 5.0;           //psi value of transducer being used
float filterPressVal = 0.0;                        //variable to store the value coming from the pressure transducer
const int numFilterReadings = 10;
int readIndexFilter = 0.0;
float totalFilter = 0.0;
float readingsFilter[numFilterReadings];
float avgFilter = 0.0;

// MCU TTEMPERATURE VARIABLES
int cpuTemp = 0;
const int numCpuReadings = 10;
int readingsCpuTemp[numCpuReadings];
int readIndexCpu = 0;                 
unsigned long totalCpuTemp = 0;                   
float avgCpuTemp = 0;
int cpuTempVal = 0;

// PushButton Variables
bool buttonOneStatus=false;
bool buttonTwoStatus=false;
bool buttonThreeStatus=false;
bool buttonFourStatus=false;
bool buttonOne=false;
bool buttonTwo=false;
bool buttonThree=false;
bool buttonFour=false;

// Status indicator varibales
int statusIndicator1 = 0;
int statusIndicator2 = 0;
int statusIndicator3 = 0;
int statusIndicator4 = 0;

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

// Initialize timer variables
unsigned long startTime, seconds, stableTime, lastTimeUpdateMillis, tempInterval, feedInterval, waterInterval, relayOneRunTime = 0;
bool resetStopWatch = false;
char strTime[18] = {0};    

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];

// variable for the IP reported when you connect to your homes intranet (during debug mode)
IPAddress Actual_IP;

// definitions of your desired intranet created by the ESP32
IPAddress PageIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);
IPAddress ip;

WebServer server(80);
TFMPlus tfmpLiquid;                                   // Create a TFMini Plus object for liquid level
TFMPlus tfmpSolid;                                    // Create a TFMini Plus object for solid level
OneWire oneWire(oneWireBus);                          // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);                  // Pass our oneWire reference to Dallas Temperature sensor 
StopWatch relayOne;

void IRAM_ATTR push_but1_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
    buttonOneStatus=!buttonOneStatus; 
    digitalWrite(2, buttonOneStatus);
    last_button_time = button_time;
    }
}

void IRAM_ATTR push_but2_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      buttonTwoStatus=!buttonTwoStatus;
      digitalWrite(18, buttonTwoStatus); 
      last_button_time = button_time;
    }
}
void IRAM_ATTR push_but3_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      resetVariables();
      last_button_time = button_time;     
    }
}
void IRAM_ATTR push_but4_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      buttonFourStatus = !buttonFourStatus;
      senderOverride = buttonFourStatus;
      digitalWrite(21, buttonFourStatus); 
      last_button_time = button_time;
    }
}


void setup() {
  Serial.begin(115200);
  Serial1.begin(115200, SERIAL_8N1, 32, 33);
  Serial2.begin(115200, SERIAL_8N1, 16, 17);
  delay(20);
  tfmpLiquid.begin( &Serial2);
  tfmpLiquid.sendCommand( SOFT_RESET, 0);
  delay(500);
  tfmpSolid.begin( &Serial1);
  tfmpSolid.sendCommand( SOFT_RESET, 0);
  delay(500);
  sensors.begin();                                    // Start the DS18B20 sensor
  pinMode(2, OUTPUT);         // Pushbutton LED for Switch 1
  pinMode(18, OUTPUT);        // Pushbutton LED for Switch 2
  pinMode(19, OUTPUT);        // Pushbutton LED for RESET
  pinMode(21, OUTPUT);        // Pushbutton LED for OVERRIDE
  pinMode(25, INPUT_PULLUP);  // Input Pushbutton 1
  pinMode(26, INPUT_PULLUP);  // Input Pushbutton 2
  pinMode(27, INPUT_PULLUP);  // Input Pushbutton 3
  pinMode(14, INPUT_PULLUP);  // Input Pushbutton 4
  attachInterrupt(25, push_but1_ISR, FALLING);
  attachInterrupt(26, push_but2_ISR, FALLING);
  attachInterrupt(27, push_but3_ISR, FALLING);
  attachInterrupt(14, push_but4_ISR, FALLING);
  // Initialize smoothing arrays to 0
  for (int thisReading = 0; thisReading < numCpuReadings; thisReading++) {
    readingsFilter[thisReading] = 0.0;}
  startTime = millis();
  // if your web page or XML are large, you may not get a call back from the web page
  // and the ESP will think something has locked up and reboot the ESP
  // not sure I like this feature, actually I kinda hate it
  // disable watch dog timer 0
  disableCore0WDT();

  // maybe disable watch dog timer 1 if needed
  //  disableCore1WDT();

  // just an update to progress
  //Serial.println("starting server");

  // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    //Serial.print(".");
  }
  //Serial.print("IP address: "); 
  //Serial.println(WiFi.localIP());
  Actual_IP = WiFi.localIP();
#endif

  // if you don't have #define USE_INTRANET, here's where you will creat and access point
  // an intranet with no internet connection. But Clients can connect to your intranet and see
  // the web page you are about to serve up
#ifndef USE_INTRANET
  WiFi.softAP(AP_SSID, AP_PASS);
  delay(100);
  WiFi.softAPConfig(PageIP, gateway, subnet);
  delay(100);
  Actual_IP = WiFi.softAPIP();
  //Serial.print("IP address: "); 
  //Serial.println(Actual_IP);
#endif

  printWifiStatus();
  
  server.on("/", SendWebsite);
  server.on("/xml", SendXML);
  server.on("/buttonOne", processButtonOne);
  server.on("/buttonTwo", processButtonTwo);
  server.on("/buttonThree", processButtonThree);
  server.on("/buttonFour", processButtonFour);
  server.begin();
}

void loop() {
  
  triggerCheck();
  if ((millis() - SensorUpdate) >= 1000) {
    SensorUpdate = millis();
    level_XML = liquidCalib();
    temp_XML = ds18b20Calib();
    solid_XML = solidCalib();
    press_XML = 2.3;
    tds_XML = 350;
    cpuTempVal = cpuTempCalib();
  }
  server.handleClient();
  if(buttonThreeStatus) {delay(1000); buttonThreeStatus = false; digitalWrite(19, LOW);} 
  // Timer opereations
  if(millis() - lastTimeUpdateMillis > 1000) {
    timeToString(strTime, sizeof(strTime));
    lastTimeUpdateMillis = millis();;
  } 
}

int cpuTempCalib() {
  cpuTemp = (temprature_sens_read() - 32) / 1.8;
  //The following code is to calculate the rolling average of the last 10 readings...
  totalCpuTemp = totalCpuTemp - readingsCpuTemp[readIndexCpu];
  readingsCpuTemp[readIndexCpu] = cpuTemp;
  totalCpuTemp = totalCpuTemp + readingsCpuTemp[readIndexCpu];
  readIndexCpu = readIndexCpu + 1;
  if (readIndexCpu >= numCpuReadings) { readIndexCpu = 0; }
  avgCpuTemp = totalCpuTemp / numCpuReadings;
  return avgCpuTemp;
}

void processButtonOne() {

  buttonOneStatus=!buttonOneStatus;
  digitalWrite(2, buttonOneStatus);
  server.send(200, "text/plain", ""); 
}

void processButtonTwo() {

  buttonTwoStatus=!buttonTwoStatus;
  digitalWrite(18, buttonTwoStatus);
  server.send(200, "text/plain", ""); 
}

void processButtonThree() {
  resetVariables();
  server.send(200, "text/plain", ""); 
}

void processButtonFour() {

  buttonFourStatus=!buttonFourStatus;
  senderOverride = buttonFourStatus;
  digitalWrite(21, buttonFourStatus);
  server.send(200, "text/plain", ""); 
}

void timeToString(char* string, size_t size)  {  
  
  if (buttonOneStatus)  {  relayOne.start(); }
  if (!buttonOneStatus)  {  relayOne.stop();  }
  relayOneRunTime = relayOne.elapsed();
  if (resetStopWatch)  {
    relayOne.reset();
    relayOneRunTime = 0;
    resetStopWatch = false;
  }
  seconds = (relayOneRunTime) / 1000;
  byte days = seconds /86400;
  seconds %= 86400;
  byte hours = seconds / 3600;
  seconds %= 3600;
  byte minutes = seconds / 60;
  seconds %= 60;
  snprintf(string, size, "%dD:%02dH:%02dM:%02dS", days, hours, minutes, seconds);
}
// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h
void SendWebsite() {
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(800, "text/html", PAGE_MAIN);
}

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
void SendXML() {
  // Serial.println("sending xml");
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");
  // send level_XML
  sprintf(buf, "<G>%d</G>\n", level_XML);
  strcat(XML, buf);
  // send temp_XML
  sprintf(buf, "<G>%4.1f</G>\n", temp_XML);
  strcat(XML, buf);
  // send feed_XML
  sprintf(buf, "<G>%d</G>\n", solid_XML);
  strcat(XML, buf);
  // send press_XML
  sprintf(buf, "<G>%f</G>\n", press_XML);
  strcat(XML, buf);
  // send tds_XML
  sprintf(buf, "<G>%d</G>\n", tds_XML);
  strcat(XML, buf);
  // send SWITCH 1 status
  if (buttonOneStatus) {
    strcat(XML, "<SW>1</SW>\n");
    }
      else {
    strcat(XML, "<SW>0</SW>\n");
    }
  // send SWITCH 2 status
  if (buttonTwoStatus) {
    strcat(XML, "<SW>1</SW>\n");
    }
    else {
        strcat(XML, "<SW>0</SW>\n");
    }
  // send SWITCH 3 status
  if (buttonThreeStatus || digitalRead(19)) {
    buttonThreeStatus = !buttonThreeStatus;
    strcat(XML, "<SW>1</SW>\n");
    }
    else {
        strcat(XML, "<SW>0</SW>\n");
    }
  // send SWITCH 4 status
  if (buttonFourStatus) {
    strcat(XML, "<SW>1</SW>\n");
    }
    else {
        strcat(XML, "<SW>0</SW>\n");
    }
  // send Status Indicator #1 (Liquid Level)
  sprintf(buf, "<SI>%d</SI>\n", statusIndicator1);
  strcat(XML, buf);
  // send Status Indicator #2 (Temperature)
  sprintf(buf, "<SI>%d</SI>\n", statusIndicator2);
  strcat(XML, buf);
  // send Status Indicator #3 (TDS Value)
  sprintf(buf, "<SI>%d</SI>\n", statusIndicator3);
  strcat(XML, buf);
  // send Status Indicator #4 (Feeder Level)
  sprintf(buf, "<SI>%d</SI>\n", statusIndicator4);
  strcat(XML, buf);
  // send datatable values for dashboard
  sprintf(buf, "<DT>%4.1f</DT>\n", maxTemp);
  strcat(XML, buf);
  sprintf(buf, "<DT>%d</DT>\n", cpuTempVal);
  strcat(XML, buf);
  sprintf(buf, "<DT>%s</DT>\n", strTime);
  strcat(XML, buf);
  // End of XML file
  strcat(XML, "</Data>\n");
  server.send(200, "text/xml", XML);
  }
// Function to calculate Liquid level from the UART Waterproof distance sensor connected to Serial2.
float liquidCalib() {
  tfmpLiquid.getData(tfDist);
  level_dist = map(tfDist,0,50,0,1300);
  if(level_dist >= 1300)  {level_dist = 1300; }
  delay(10); 
  return level_dist; 
}
// Function to calculate Solid level from the UART Waterproof distance sensor connected to Serial3.
float solidCalib() {
  tfmpSolid.getData(tfDist);
  solid_dist = map(tfDist,0,50,0,100);
  if(solid_dist >= 100)  {solid_dist = 100; }
  delay(10); 
  return solid_dist; 
}
// Function to calculate Liquid Temperature  using the DS18B20 Waterproof Temperature Sensor
// (connected to pin 4) using a smoothing technic on a rolling average of last 10 readings.
float ds18b20Calib() {
  sensors.requestTemperatures();
  tempObjecC = sensors.getTempC(tempSensor1);
  if(tempObjecC > maxTemp) {maxTemp = tempObjecC; }
  if (tempObjecC < -10) {tempObjecC = -10;}
  if (tempObjecC > 40) {tempObjecC = 40; }
  return tempObjecC;
}
void triggerCheck()
{   
    // Liquid Level  
    if  (level_XML >= 1100) {
        warnConsistFull++;
        warnConsistEmpty=0;
        }
    if  (level_XML <= 1099) {
        warnConsistEmpty++;
        warnConsistFull=0;
        }
    if  (warnConsistEmpty > 10 && !senderOverride) {
        warnConsistEmpty=0;
          if  (!smsLevelOverride) { 
          HTTPClient http;
          http.begin(eventLiquid);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(level_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsLevelOverride = true;
          http.end();
          }
        }
        if (level_XML < 1099) { statusIndicator1 = 0;}
        if (level_XML > 1099) { statusIndicator1 = 1; }

    // Temperature Level
    if  (temp_XML >= 28) {
        tempConsistHot++;
        tempConsistCold=0;
        }
    if  (temp_XML < 28) {
        tempConsistCold++;;
        tempConsistHot=0;
        }
    if  (tempConsistHot > 10 && !senderOverride) {
        tempConsistHot=0;
        if  (!smsTempOverride) { 
          HTTPClient http;
          http.begin(eventTemp);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(temp_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsTempOverride = true;
          http.end();
          }
        }
        if (temp_XML < 28) { statusIndicator2 = 1;}
        if (temp_XML >= 28) { statusIndicator2 = 0; }
  
    // SOLID LEVEL 
    if  (solid_XML > 10) {
        solidConsistFull++;
        solidConsistLow=0;
        }
    if  (solid_XML <= 10) {
        solidConsistLow++;
        solidConsistFull=0;
        }
    if  (solidConsistLow > 10 && !senderOverride) {
        solidConsistLow=0;
          if  (!smsSolidOverride) { 
          HTTPClient http;
          http.begin(eventSolid);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(solid_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsSolidOverride = true;
          http.end();
          }
        }
        if (solid_XML <= 10) { statusIndicator4 = 0;}
        if (solid_XML > 10) { statusIndicator4 = 1; }

    // TDS Quality Warning
    if  (tds_XML > 500) {
        tdsConsistHigh++;
        tdsConsistLow=0;
        }
    if  (tds_XML <= 500) {
        tdsConsistLow++;
        tdsConsistHigh=0;
        }
    if  (tdsConsistHigh > 10 && !senderOverride) {
        tdsConsistHigh=0;
          if  (!smsTdsOverride) { 
          HTTPClient http;
          http.begin(eventTds);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(tds_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsTdsOverride = true;
          http.end();
          }
        }
        if (tds_XML < 500) { statusIndicator3 = 1;}
        if (tds_XML >= 500) { statusIndicator3 = 0; }

        
}
void resetVariables() {
  digitalWrite(19,HIGH);
  if(buttonOneStatus) {buttonOneStatus = false;}
  if(buttonTwoStatus) {buttonTwoStatus = false;}
  if(buttonFourStatus){buttonFourStatus = false; senderOverride = false;}
  smsSolidOverride, smsTempOverride, smsTdsOverride, smsLevelOverride, smsWarnOverride = false;  
  digitalWrite(2,LOW);
  digitalWrite(18, LOW);
  digitalWrite(21, LOW);
  startTime = millis();
  resetStopWatch = true;
  maxTemp = 0;
}
void printWifiStatus() {

  // print the SSID of the network you're attached to:
  //Serial.print("SSID: ");
  //Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  //Serial.print("IP Address: ");
  //Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  //Serial.print("signal strength (RSSI):");
  //Serial.print(rssi);
  //Serial.println(" dBm");
  // print where to go in a browser:
  //Serial.print("Open http://");
  //Serial.println(ip);
}
