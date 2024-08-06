/*--------------------------------------------------------------
  Program:      Pond Monitoring and Automation System Dashboard
  Version:      v1.0.4b
  Description:  This C++ code works on the ESP32 microprocessor in conjunction with a web server web page
                to display a dashboard containing 6 analalog Canvas-Gauge dials, 8 numerical data points, 2 interactive toggle switches 
                which control relays 1 & 2, two interactive pushbutton type switches to control a reset and override function as well as
                6 graphical status indicators.
                The main web page is stored in a large string variable stored the ESP32's memory, graphics and larger js files are stored on a CDN. 
                Ajax is used to update the analog values of the Gauges, the table data and the toggle switches associated with the webpage in realtime. 
                There is some customizable intelligence built into the program which triggers relays should sensors supply data outside nominal 
                operating parameters as well as send rich text notifications to the operators smartphone of specific issue encountered. 
  
  Hardware:     Espressif ESP32 compatible board (ex: ESP32-DevKitC-32UE) 
                Also required is a 2 channel 5VDC DIN Rail relay, an external power supply and other components such as specialized sensors,
                bus extenders and mounting hardware based on specific needs.                
                
  Software:     Developed using Arduino 2.1.0 software
                
  
  References:   - Canvas-Gauge from:
                  https://github.com/Mikhus/canv-gauge
                - Stopwatch_RT Library by Rob Tillaart
                  https://github.com/RobTillaart/Stopwatch_RT

  Date:         July 15th, 2024
 
  Author:       Richard Inniss, http://aiotconsulting.com
--------------------------------------------------------------*/
String firmware = "v1.0.4b";
#include <WiFi.h>             
#include <WiFiManager.h>
#include <WebServer.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFMPlus.h>                    // Include TFMini Plus Library v1.5.0
#include <StopWatch.h>
#include <Ticker.h>
#include "web_interface_1.4.h"         // .h file that stores our webpage
#include <HTTPClient.h>
#include<HardwareSerial.h>
#include <NewPing.h>

// Individual SMS event triggers
const char* eventLiquid = "https://maker.ifttt.com/trigger/Liquid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventSolid = "https://maker.ifttt.com/trigger/Solid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTemp = "https://maker.ifttt.com/trigger/Temp_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTds = "https://maker.ifttt.com/trigger/Tds_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventIp = "https://maker.ifttt.com/trigger/Ip_Address/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";


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
bool senderOverride = false; 
bool smsSolidOverride, smsTempOverride, smsTdsOverride, smsLevelOverride;
bool altFunc, trigger_Pressed = false;
uint32_t SensorUpdate = 0;

// LIQUID LEVEL VARIABLES
float duration; 
float distance; 
int gallons;
int actualGals;
int level_XML = 0;
int levelConsistFull = 0;
int levelConsistEmpty = 0;
const int numGalsReadings = 5;
int readingsGals[numGalsReadings];
int readIndexGals = 0;                 
unsigned long totalGals = 0;                   
float avgGals = 0;

// TEMP SENSOR VARIABLES:
#define ONE_WIRE_BUS 15
float temp_XML = 0;
byte tempSensor1[] = {0x28, 0xAA, 0x90, 0xAD, 0x0F, 0x00, 0x00, 0x51};  // Unique DS18B20 address - need to change this for each sensor
int tempConsistHot=0;
int tempConsistCold=0;
float tempObjecC = 0.0;
float maxTemp=0.0;
int maxTempCount=0;
float previousTemp = 0.0;

// TDS SENSOR VARIABLES:
int tds_XML = 0;
float temperature = 25;
#define VREF 3.3                                  // analog reference voltage(Volt) of the ADC
#define SCOUNT  30                                // sum of sample point
int analogBuffer[SCOUNT];                         // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0;
int tdsConsistHigh = 0;
int tdsConsistLow = 0;

// SOLID LEVEL VARIABLES
int solid_XML = 0;
int solid_dist = 0; 
int16_t tfDist2 = 0;
const int numSolidReadings = 10;
float readingsSolid[numSolidReadings];
int readIndexSolid = 0;                 
float totalSolid = 0.0;                   
float avgSolid = 0.0;                                        
int solidConsistFull=0;
int solidConsistLow=0;

// PRESSURE SENSOR VARIABLES:
const int numAirReadings = 10;
float readingsAir[numAirReadings];
int readIndexAir = 0;                 
float totalAir = 0.0;                   
float avgAir = 0.0;
float baselineVoltageLiqu = 0.5;
float airPress_XML = 0.0;
float liquPress_XML = 0;
float rawPress = 0.0;
float airPressure = 0;
float liquPressure = 0;
float liquPressVolts = 0.0;

// MCU TTEMPERATURE VARIABLES
int cpuTemp = 0;
const int numCpuReadings = 10;
int readingsCpuTemp[numCpuReadings];
int readIndexCpu = 0;                 
unsigned long totalCpuTemp = 0.0;                   
float avgCpuTemp = 0;
int cpuTempVal = 0;

// PushButton Variables
bool buttonOneStatus=false;
bool buttonTwoStatus=false;
bool buttonThreeStatus=false;
bool buttonFourStatus=false;
unsigned long buttonPressTime = 0;
bool currentButtonState;

// Status indicator varibales
int statusIndicator1 = 0;
int statusIndicator2 = 0;
int statusIndicator3 = 0;
int statusIndicator4 = 0;

//variables to keep track of the timing of recent interrupts
unsigned long button_time = 0;  
unsigned long last_button_time = 0; 

// Initialize timer variables
unsigned long startTime, seconds, lastTimeUpdateMillis, relayOneRunTime = 0;
bool resetStopWatch = false;
char strTime[18] = {0};    

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// just some buffer holder for char operations
char buf[32];

WebServer server(80);
HardwareSerial SerialLiquid(2);
TFMPlus tfmpSolid;                                    // Create a TFMini Plus object for solid level
OneWire oneWire(ONE_WIRE_BUS);                        // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);                  // Pass our oneWire reference to Dallas Temperature sensor 
StopWatch relayOne;
//Ticker watchDogTick;
Ticker ticker;
NewPing sonar(17, 16, 200);
//volatile int watchDogCount = 0;
#define TRIGGER_PIN 0
                                    
void IRAM_ATTR push_but1_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 750) {
    buttonOneStatus=!buttonOneStatus; 
    digitalWrite(2, buttonOneStatus);
    last_button_time = button_time;
    }
}
void IRAM_ATTR push_but2_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 750) {
      buttonTwoStatus=!buttonTwoStatus;
      digitalWrite(5, buttonTwoStatus); 
      last_button_time = button_time;
    }
}
void IRAM_ATTR push_but3_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 750) {
      resetVariables();
      last_button_time = button_time;     
    }
}
void IRAM_ATTR push_but4_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 750) {
      buttonFourStatus = !buttonFourStatus;
      senderOverride = buttonFourStatus;
      digitalWrite(21, buttonFourStatus); 
      last_button_time = button_time;
    }
}

void setup() {
  WiFi.mode(WIFI_STA);
  Serial.begin(115200);
  delay(200);
  Serial1.begin(115200, SERIAL_8N1, 32, 33);
  tfmpSolid.begin( &Serial1);
  tfmpSolid.sendCommand( SOFT_RESET, 0);
  delay(200);
  SerialLiquid.begin(9600, SERIAL_8N1, 16, 17); 
  //Serial2.begin(9600, SERIAL_8N1, 16, 17);  
  //tfmpLiquid.begin( &Serial2);
  //tfmpLiquid.sendCommand( SOFT_RESET, 0);
  
  //watchDogTick.attach(1, isrWatchdog);
  sensors.begin();                                    // Start the DS18B20 sensor
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(2, OUTPUT);         // Relay Input & Pushbutton LED for Switch 1
  pinMode(5, OUTPUT);         // Relay Input & Pushbutton LED for Switch 2
  pinMode(4, OUTPUT);         // WiFi Connection LED
  pinMode(15, INPUT);         // For OneWire Bus
  pinMode(16, INPUT);         // A02YYUW Waterproof Ultrasonic Sensor trigger pin
  pinMode(17, OUTPUT);        // A02YYUW Waterproof Ultrasonic Sensor echo pin
  digitalWrite(17, LOW);
  pinMode(19, OUTPUT);        // Pushbutton LED for RESET
  pinMode(21, OUTPUT);        // Pushbutton LED for OVERRIDE
  pinMode(36,INPUT_PULLUP);   // Input Pin for Air Pressure sensor
  pinMode(35,INPUT_PULLUP);   // Input Pin for TDS sensor
  //adcAttachPin(35);
  pinMode(39, INPUT_PULLUP);  // Input Pin for Liquid Pressure Sensor *** pins 34 to 39 are inputs only and do not have pullup capabilties
  pinMode(25, INPUT_PULLUP);  // Input Pushbutton 1
  pinMode(26, INPUT_PULLUP);  // Input Pushbutton 2
  pinMode(27, INPUT_PULLUP);  // Input Pushbutton 3
  pinMode(14, INPUT_PULLUP);  // Input Pushbutton 4
  attachInterrupt(25, push_but1_ISR, FALLING);
  attachInterrupt(26, push_but2_ISR, FALLING);
  attachInterrupt(27, push_but3_ISR, FALLING);
  attachInterrupt(14, push_but4_ISR, FALLING);
  // Initialize smoothing arrays to 0
  for (int thisReading = 0; thisReading < numGalsReadings; thisReading++) {
    readingsGals[thisReading] = 0.0;}
  for (int thisReading = 0; thisReading < numCpuReadings; thisReading++) {
    readingsCpuTemp[thisReading] = 0.0;}
  for (int thisReading = 0; thisReading < numAirReadings; thisReading++) {
    readingsAir[thisReading] = 0.0;}
  for (int thisReading = 0; thisReading < numSolidReadings; thisReading++) {
    readingsSolid[thisReading] = 0.0;}
  startTime = millis();
  ticker.attach(0.9, tick);
  WiFiManager wm;
  wm.setAPCallback(configModeCallback);
  wm.setClass("invert"); 
  wm.setScanDispPerc(true); 
  wm.setMinimumSignalQuality(40);
  wm.setConfigPortalTimeout(120);
  //wm.resetSettings();
  std::vector<const char *> menu = {"wifi","info","update","exit"};
  wm.setMenu(menu);
  WiFiManagerParameter ifttt_api_key("ifttt_key", "Enter you IFTTT API Key Here", "", 50);
  wm.addParameter(&ifttt_api_key);
  if(!wm.autoConnect("Saturn V")) {
    Serial.println("failed to connect and hit timeout");
    //rest and try again, or maybe put it in deep sleep
    ESP.restart();
    delay(1000);
  }
  else  {
    Serial.println("Connected...");    
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.print("ifttt_key: ");
    Serial.println(ifttt_api_key.getValue());
    String trigger_key = ifttt_api_key.getValue();
    unsigned int key_length = trigger_key.length();
    ticker.detach();
    digitalWrite(4, HIGH);
    if(key_length > 0) {ip_Add_Trigger();}
  }
  server.on("/", SendWebsite);
  server.on("/xml", SendXML);
  server.on("/buttonOne", processButtonOne);
  server.on("/buttonTwo", processButtonTwo);
  server.on("/buttonThree", processButtonThree);
  server.on("/buttonFour", processButtonFour);
  server.begin();
}

void loop() {
  // Process the WiFi Reset when WiFi Reset button is Press for more then 5 Sec.
  currentButtonState = digitalRead(TRIGGER_PIN);
  if (!currentButtonState) {
    buttonPressTime = millis();
    while (!digitalRead(TRIGGER_PIN));
    if (millis() - buttonPressTime > 5000) {WiFiManager wm; wm.resetSettings(); ESP.restart();} 
  }
  //watchDogCount = 0;
  //triggerCheck();
  if ((millis() - SensorUpdate) >= 2000) {
    altFunc = !altFunc;
    SensorUpdate = millis();
    //level_XML = liquidCalib();
    if(altFunc) {temp_XML = ds18b20Calib();}
    if(!altFunc) {solid_XML = solidCalib();}
    liquPress_XML = liquPressCalib();
    airPress_XML = airPressCalib();
    //tds_XML = tdsCalib();
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

void processButtonOne() {

  buttonOneStatus=!buttonOneStatus;
  digitalWrite(2, buttonOneStatus);
  server.send(200, "text/plain", ""); 
}

void processButtonTwo() {

  buttonTwoStatus=!buttonTwoStatus;
  digitalWrite(5, buttonTwoStatus);
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
  server.send(200, "text/html", PAGE_MAIN);
}
// code to send the main web page
void SendXML() {
  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");
  // send level_XML
  sprintf(buf, "<G>%d</G>\n", level_XML);
  strcat(XML, buf);
  // send temp_XML
  sprintf(buf, "<G>%4.1f</G>\n", temp_XML);
  strcat(XML, buf);
  // send liquPress_XMLL
  sprintf(buf, "<G>%4.1f</G>\n", liquPress_XML);
  strcat(XML, buf);
  // send airPress_XML
  sprintf(buf, "<G>%4.1f</G>\n", airPress_XML);
  strcat(XML, buf);
  // send feed_XML
  sprintf(buf, "<G>%d</G>\n", solid_XML);
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
  sprintf(buf, "<DT>%4.1f</DT>\n", liquPressure);
  strcat(XML, buf);
  sprintf(buf, "<DT>%4.1f</DT>\n", airPressure);
  strcat(XML, buf);
  sprintf(buf, "<DT>%d</DT>\n", actualGals);
  strcat(XML, buf);
  sprintf(buf, "<DT>%d</DT>\n", cpuTempVal);
  strcat(XML, buf);
  sprintf(buf, "<DT>%s</DT>\n", strTime);
  strcat(XML, buf);
  // send Firmware version
  sprintf(buf, "<FW>%s</FW>\n", firmware);
  strcat(XML, buf);
  strcat(XML, "</Data>\n");
  // End of XML file
  //Serial.println(XML);
  server.send(400, "text/xml", XML);
  }

// Function to calculate Liquid level from the UART Waterproof distance LIDAR sensor connected to Serial2.
float liquidCalib() {
    duration = sonar.ping();
    distance =((duration / 2) * 0.0350 / 2.54); 
    gallons = (-40.431 * distance) +1455.5;    // Linear equation based on water distance being at 6.32 inches when full @ 1300 gallons and at 36 inches when empty: y = -43.801x + 1576.8
    actualGals = gallons;
    if (gallons >= 1300) {gallons = 1300;}
    if (gallons <= 1) {gallons = 0;} 
    //The following code is to calculate the rolling average of the last 10 readings...
    totalGals = totalGals - readingsGals[readIndexGals];
    readingsGals[readIndexGals] = gallons;
    totalGals = totalGals + readingsGals[readIndexGals];
    readIndexGals = readIndexGals + 1;
    if (readIndexGals >= numGalsReadings) { readIndexGals = 0; }
    avgGals = totalGals / numGalsReadings;
return avgGals;
}

// Function to calculate Solid level from the UART Waterproof distance LIDAR sensor connected to Serial3.
float solidCalib() {
  if(tfmpSolid.getData(tfDist2))  {
  solid_dist = map(tfDist2,17,1,0,100);         // On the feeder, the sensors reading is 17 cm's when container almost empty and 1 cm when almost full...
  if(solid_dist >= 100)  {solid_dist = 100; }
  if(solid_dist <= 0)  {solid_dist = 0; }
  delay(10); 
  } else  {
  solid_dist = 0;
  }
  //The following code is to calculate the rolling average of the last 10 readings...
  totalSolid = totalSolid - readingsSolid[readIndexSolid];
  readingsSolid[readIndexSolid] = solid_dist;
  totalSolid = totalSolid + readingsSolid[readIndexSolid];
  readIndexSolid = readIndexSolid + 1;
  if (readIndexSolid >= numSolidReadings) { readIndexSolid = 0; }
  avgSolid = totalSolid / numSolidReadings;
return avgSolid; 
}

// Function to calculate Liquid Temperature  using the DS18B20 Waterproof Temperature Sensor
float ds18b20Calib() {
  previousTemp = tempObjecC;
  sensors.requestTemperatures();
  tempObjecC = sensors.getTempC(tempSensor1);
  if(tempObjecC == -127) {tempObjecC = previousTemp; }
  if(tempObjecC > maxTemp) {
    maxTempCount++;
      if(maxTempCount > 5)  {
        maxTemp = tempObjecC;
        maxTempCount = 0;
      }
  }
  if (tempObjecC < -10) {tempObjecC = -10;}
  if (tempObjecC > 40) {tempObjecC = 40; }
  return tempObjecC;
}

// Function to calculate pressure of of water oxygen pump 
// values from the 5 PSI pressure sensor (connected to pin 36) 
float liquPressCalib()  {
  rawPress = (float)analogRead(39);
  liquPressVolts = rawPress * (3.3 / 4096); 
  liquPressure = (liquPressVolts - baselineVoltageLiqu) * (5.0 / (3.3 - baselineVoltageLiqu));
  if (liquPressure >= 10) {liquPressure = 10;}
  if (liquPressure <= 0.5) {liquPressure = 0;} 
  
  return liquPressure;
}

// Function to calculate pressure of of water filtration system 
// values from the 5 PSI pressure sensor (connected to pin 36) 
float airPressCalib()  {
  rawPress = (float)analogRead(36);
  airPressure = ((0.0013*rawPress)- 0.0794)*2.5;    // Linear Equation based on measured data is y = 0.0013x-0.0794
  if (airPressure >= 5) {airPressure = 5;}
  if (airPressure <= 0.1) {airPressure = 0;} 
  //The following code is to calculate the rolling average of the last 10 readings...
  totalAir = totalAir - readingsAir[readIndexAir];
  readingsAir[readIndexAir] = airPressure;
  totalAir = totalAir + readingsAir[readIndexAir];
  readIndexAir = readIndexAir + 1;
  if (readIndexAir >= numAirReadings) { readIndexAir = 0; }
  avgAir = totalAir / numAirReadings;
return avgAir;
}


// Function to calculate liquid quality TDS (Total Dissolved Solids) in ppm for water
float tdsCalib() {
    analogBuffer[analogBufferIndex] = analogRead(35);    //read the analog value and store into the buffer
    analogBufferIndex++;
    if(analogBufferIndex == SCOUNT){ 
      analogBufferIndex = 0;
    }
    for(copyIndex=0; copyIndex < SCOUNT; copyIndex++){
      analogBufferTemp[copyIndex] = analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 4096.0;
      float compensationCoefficient = 1.0+0.02*(temperature-25.0);
      float compensationVoltage=averageVoltage/compensationCoefficient;
      tdsValue = (133.42*compensationVoltage*compensationVoltage*compensationVoltage - 255.86*compensationVoltage*compensationVoltage + 857.39*compensationVoltage)*0.5;
      if(tdsValue < 0)  {tdsValue = 0;}
   }
  return tdsValue;
}
// Used for the tdsCalib() function (median filtering algorithm)
int getMedianNum(int bArray[], int iFilterLen){
  int bTab[iFilterLen];
  for (byte i = 0; i<iFilterLen; i++)
  bTab[i] = bArray[i];
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {bTemp = bTab[i]; bTab[i] = bTab[i + 1]; bTab[i + 1] = bTemp;}
    }
  }
  if ((iFilterLen & 1) > 0){bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
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
void triggerCheck()
{   
    // Liquid Level  
    if  (level_XML >= 1100) {
        levelConsistFull++;
        levelConsistEmpty=0;
        }
    if  (level_XML <= 1099) {
        levelConsistEmpty++;
        levelConsistFull=0;
        }
    if  (levelConsistEmpty > 10 && !senderOverride) {
        levelConsistEmpty=0;
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
  buttonThreeStatus = true;
  if(buttonFourStatus){buttonFourStatus = false; senderOverride = false;}
  smsSolidOverride, smsTempOverride, smsTdsOverride, smsLevelOverride = false;  
  digitalWrite(2,LOW);
  digitalWrite(5, LOW);
  digitalWrite(21, LOW);
  startTime = millis();
  resetStopWatch = true;
  maxTemp = 0;
}
void ip_Add_Trigger() {
          HTTPClient http;
          http.begin(eventIp);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + WiFi.localIP().toString() + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsSolidOverride = true;
          http.end();
          }
void tick()
{
  digitalWrite(4, !digitalRead(4));    
}
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

/*
void isrWatchdog()  {
  watchDogCount++;
  if(watchDogCount == 60)  {
    Serial.println();
    Serial.println("The WatchDog has been fired....");
    ESP.restart();
  }
}*/
