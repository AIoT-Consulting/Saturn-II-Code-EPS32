#include <WiFi.h>             // standard library
#include <WiFiManager.h>
#include <Preferences.h>
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFMPlus.h>  // Include TFMini Plus Library v1.5.0
#include <StopWatch.h>
#include <Ticker.h>
#include "web_interface_1.3.h"         // .h file that stores our webpage interface
#include <HTTPClient.h>

// Individual SMS event triggers
const char* eventLiquid = "https://maker.ifttt.com/trigger/Liquid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventSolid = "https://maker.ifttt.com/trigger/Solid_Low/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTemp = "https://maker.ifttt.com/trigger/Temp_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventTds = "https://maker.ifttt.com/trigger/Tds_High/with/key/kg0Hq89vMVBXu0RtTG-g7a-uyr3LFWICCROFKN2Ieze";
const char* eventIp = "Ip_Address";

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
float voltage = 3.3, ecValue, temperature = 25;
#define VREF 3.3                                  // analog reference voltage(Volt) of the ADC
#define SCOUNT  30                                // sum of sample point
int analogBuffer[SCOUNT];                         // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0, copyIndex = 0;
float averageVoltage = 0, tdsValue = 0, tdsVal = 0;
String tdsString = "999"; 
int tdsConsistHigh = 0;
int tdsConsistLow = 0;

// SOLID LEVEL VARIABLES
uint8_t feed_buf[9] = {0};                                
int solid_XML = 0;
int solid_dist = 0;                                        
int solidLevelVal = 0;
int solidConsistFull=0;
int solidConsistLow=0;
String solidString = "999";
int soldiLevel; 

// PRESSURE SENSOR VARIABLES:
float press_XML = 0;
int rawPress = 0;
float pressZero = 170.0;                       //analog reading of pressure transducer at 0psi
float pressMax = 1023.0;                       //analog reading of pressure transducer at 100psi
float ratedMaxPSI = 10.0;                      //psi value of transducer being used
const int numPressReadings = 10;
int readIndexPress = 0.0;
float totalPress = 0.0;
float readingsPress[numPressReadings];
float avgPress = 0.0;
float pressure = 0;

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
unsigned long startTime, seconds, stableTime, lastTimeUpdateMillis, tempInterval, feedInterval, waterInterval, relayOneRunTime = 0;
bool resetStopWatch = false;
char strTime[18] = {0};    

// the XML array size needs to be bigger that your maximum expected size. 2048 is way too big for this example
char XML[2048];

// the IFTTT api key that will be passed by the WiFi Manager upon intial setup
//const char* IFTTT_key;
String IFTTT_key;
bool key_Reset = false;

// just some buffer holder for char operations
char buf[32];

WebServer server(80);
TFMPlus tfmpLiquid;                                   // Create a TFMini Plus object for liquid level
TFMPlus tfmpSolid;                                    // Create a TFMini Plus object for solid level
OneWire oneWire(oneWireBus);                          // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);                  // Pass our oneWire reference to Dallas Temperature sensor 
StopWatch relayOne;
//Ticker watchDogTick;
Ticker ticker;
Preferences ifttt;
//volatile int watchDogCount = 0;
#define TRIGGER_PIN 0
                                    
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
      digitalWrite(5, buttonTwoStatus); 
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
      digitalWrite(19, buttonFourStatus); 
      last_button_time = button_time;
    }
}

void setup() {
  WiFi.mode(WIFI_STA);
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
  
  //watchDogTick.attach(1, isrWatchdog);
  sensors.begin();                                    // Start the DS18B20 sensor
  pinMode(TRIGGER_PIN, INPUT_PULLUP);
  pinMode(2, OUTPUT);         // Pushbutton LED for Switch 1
  pinMode(5, OUTPUT);        // Pushbutton LED for Switch 2
  pinMode(4, OUTPUT);         // WiFi Connection LED
  pinMode(18, OUTPUT);        // Pushbutton LED for RESET
  pinMode(19, OUTPUT);        // Pushbutton LED for OVERRIDE
  pinMode(36,INPUT_PULLUP);   // Input Pin for TDS sensor
  pinMode(34, INPUT_PULLUP);  // Input Pin for Pressure Sensor
  pinMode(25, INPUT_PULLUP);  // Input Pushbutton 1
  pinMode(26, INPUT_PULLUP);  // Input Pushbutton 2
  pinMode(27, INPUT_PULLUP);  // Input Pushbutton 3
  pinMode(14, INPUT_PULLUP);  // Input Pushbutton 4
  adcAttachPin(36);
  attachInterrupt(25, push_but1_ISR, FALLING);
  attachInterrupt(26, push_but2_ISR, FALLING);
  attachInterrupt(27, push_but3_ISR, FALLING);
  attachInterrupt(14, push_but4_ISR, FALLING);
  // Initialize smoothing arrays to 0
  for (int thisReading = 0; thisReading < numCpuReadings; thisReading++) {
    readingsCpuTemp[thisReading] = 0.0;}
  for (int thisReading = 0; thisReading < numPressReadings; thisReading++) {
    readingsPress[thisReading] = 0.0;}
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
    ifttt.begin("credentials", false);
    if(ifttt.getBool("key_Reset")) {
      ifttt.putString("IFTTT_key", ifttt_api_key.getValue());
      ifttt.putBool("key_Reset", false);
      IFTTT_key = ifttt.getString("IFTTT_key", "");
      ip_Add_Trigger();
    }
    //Serial.println("Connected...");    
    //Serial.print("IP Address: ");
    //Serial.println(WiFi.localIP());
    //IFTTT_key = ifttt.getString("IFTTT_key", "");
    //Serial.print("IFTTT API Key is: ");
    //Serial.println(IFTTT_key);
    ifttt.end();
    ticker.detach();
    digitalWrite(4, HIGH);
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
    if (millis() - buttonPressTime > 5000) {
      WiFiManager wm; 
      wm.resetSettings();
      ifttt.begin("credentials", false);
      ifttt.putBool("key_Reset", true);       // This will set the value stored in memory for the key_Reset variabe to TRUE
      ifttt.remove("IFTTT_key");              // This will remove the current current IFTTT API key
      ifttt.end();
      ESP.restart();} 
  }
  
  //watchDogCount = 0;
  triggerCheck();
  if ((millis() - SensorUpdate) >= 500) {
    SensorUpdate = millis();
    level_XML = liquidCalib();
    temp_XML = ds18b20Calib();
    solid_XML = solidCalib();
    press_XML = pressCalib();
    cpuTempVal = cpuTempCalib();
    tds_XML = tdsCalib();
  }
  server.handleClient();
  if(buttonThreeStatus) {delay(1000); buttonThreeStatus = false; digitalWrite(18, LOW);} 
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
  digitalWrite(19, buttonFourStatus);
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
  sprintf(buf, "<G>%4.1f</G>\n", press_XML);
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
  if (buttonThreeStatus || digitalRead(18)) {
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
  //Serial.println(XML);
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
// Function to calculate an average pressure value over specified number of samples (to limit fluctuations) and to convert 
// values from the 100 PSI pressure sensor (connected to analog pin 5) to accurately reflect the circuit environment.
float pressCalib()  {
  rawPress = analogRead(34); 
  pressure = ((rawPress-pressZero)*ratedMaxPSI)/(pressMax-pressZero); 
  if (pressure >= 5) {pressure = 5;}
  if (pressure <= 0.5) {pressure = 0;} 
  //The following code is to calculate the rolling average of the last 10 readings...
  totalPress = totalPress - readingsPress[readIndexPress];
  readingsPress[readIndexPress] = pressure;
  totalPress = totalPress + readingsPress[readIndexPress];
  readIndexPress = readIndexPress + 1;
  if (readIndexPress >= numPressReadings) { readIndexPress = 0; }
  avgPress = totalPress / numPressReadings;
  return avgPress;
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
float ds18b20Calib() {
  sensors.requestTemperatures();
  tempObjecC = sensors.getTempC(tempSensor1);
  if(tempObjecC > maxTemp) {maxTemp = tempObjecC; }
  if (tempObjecC < -10) {tempObjecC = -10;}
  if (tempObjecC > 40) {tempObjecC = 40; }
  return tempObjecC;
}
// Function to calculate liquid quality TDS (Total Dissolved Solids) in ppm for water
float tdsCalib() {
    analogBuffer[analogBufferIndex] = analogRead(36);    //read the analog value and store into the buffer
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
        warnConsistFull++;
        warnConsistEmpty=0;
        }
    if  (level_XML <= 1099) {
        warnConsistEmpty++;
        warnConsistFull=0;
        }
    if  (warnConsistEmpty > 10 && !senderOverride) {
        warnConsistEmpty=0;
          /*if  (!smsLevelOverride) { 
          HTTPClient http;
          http.begin(eventLiquid);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(level_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsLevelOverride = true;
          http.end();
          }*/
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
        /*if  (!smsTempOverride) { 
          HTTPClient http;
          http.begin(eventTemp);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(temp_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsTempOverride = true;
          http.end();
          }*/
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
          /*if  (!smsSolidOverride) { 
          HTTPClient http;
          http.begin(eventSolid);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(solid_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsSolidOverride = true;
          http.end();
          }*/
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
          /*if  (!smsTdsOverride) { 
          HTTPClient http;
          http.begin(eventTds);
          delay(500);
          http.addHeader("Content-Type", "application/json");
          String httpRequestData = "{\"value1\":\"" + String(tds_XML) + "\"}";
          int httpResponseCode = http.POST(httpRequestData);
          smsTdsOverride = true;
          http.end();
          }*/
        }
        if (tds_XML < 500) { statusIndicator3 = 1;}
        if (tds_XML >= 500) { statusIndicator3 = 0; }

        
}
void resetVariables() {
  digitalWrite(18,HIGH);
  if(buttonOneStatus) {buttonOneStatus = false;}
  if(buttonTwoStatus) {buttonTwoStatus = false;}
  if(buttonFourStatus){buttonFourStatus = false; senderOverride = false;}
  smsSolidOverride, smsTempOverride, smsTdsOverride, smsLevelOverride, smsWarnOverride = false;  
  digitalWrite(2,LOW);
  digitalWrite(5, LOW);
  digitalWrite(19, LOW);
  startTime = millis();
  resetStopWatch = true;
  maxTemp = 0;
}
void ip_Add_Trigger() {
          String url = "https://maker.ifttt.com/trigger/" + String(eventIp) + "/with/key/" + String(IFTTT_key);
          HTTPClient http;
          http.begin(url);
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
