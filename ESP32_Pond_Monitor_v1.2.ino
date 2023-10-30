

#include <WiFi.h>             // standard library
#include <WebServer.h>        // standard library
#include <SPI.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <TFMPlus.h>  // Include TFMini Plus Library v1.5.0
#include "SuperMon.h"         // .h file that stores your html page code

// here you post web pages to your homes intranet which will make page debugging easier
// as you just need to refresh the browser as opposed to reconnection to the web server
#define USE_INTRANET

// replace this with your homes intranet connect parameters
#define LOCAL_SSID "RatNet"
#define LOCAL_PASS "P1nouche"

// once  you are read to go live these settings are what you client will connect to
#define AP_SSID "Pond"
#define AP_PASS "Fish"

// start your defines for pins for sensors, outputs etc.
#define PIN_PUSHBUT_1 25  // Connected to the Push Button 1 LED
#define PIN_OUTPUT 26     // connected to nothing but an example of a digital write from the web page
#define PIN_A0 34         // some analog input sensor
#define PIN_A1 35         // some analog input sensor
#define RXD2 16
#define TXD2 17

// variables to store measure data and sensor states
int online = 0;
int offCount = 0;
float VoltsA0 = 0, VoltsA1 = 0;
bool senderOverride = false; 
bool smsFeedOverride, smsTempOverride, smsTdsOverride, smsLevelOverride, smsWarnOverride = false;       
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
String ltrString1 = "999";
String ltrString2 = "999";              
const int numDist1Readings = 5;
int readingsDist1[numDist1Readings];
int readIndexDist1 = 0;                 
unsigned long totalDist1 = 0;                   
float avgDist1 = 0;
float duration1;   

// TEMP SENSOR VARIABLES:
int temp_XML = 0;
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

// FISH FEEDER FOOD LEVEL VARIABLES
uint8_t feed_buf[9] = {0};                                 // An array that holds data
int feed_XML = 0;
int feed_dist = 0;                                         // The variable "distance2" will contain the distance value in millimeter from FishFeeder distance sensor
int feederLevelVal = 0;
int feedConsistFull=0;
int feedConsistLow=0;
String feedString = "999";
int feedLevel; 

// FILTER PRESSURE SENSOR VARIABLES:
float press_XML = 0;
float filterPressZero = 96.0;                      //analog reading of pressure transducer at 0psi
float filterPressMax = 139.0;                      //analog reading of pressure transducer at 100psi
float filterPresstransducermaxPSI = 5.0;           //psi value of transducer being used
float filterPressVal = 0.0;                        //variable to store the value coming from the pressure transducer
const int numFilterReadings = 10;
int readIndexFilter = 0.0;
float totalFilter = 0.0;
float readingsFilter[numFilterReadings];
float avgFilter = 0.0;

// PushButton Variables
boolean SWITCH_state[4] = {0}; 
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

// gotta create a server
WebServer server(80);
TFMPlus tfmP;                                         // Create a TFMini Plus object
OneWire oneWire(oneWireBus);                          // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature sensors(&oneWire);                  // Pass our oneWire reference to Dallas Temperature sensor 

void IRAM_ATTR push_but1_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
    buttonOneStatus=!buttonOneStatus; 
    SWITCH_state[0] = !SWITCH_state[0]; 
    digitalWrite(2, buttonOneStatus);
    last_button_time = button_time;
    }
}

void IRAM_ATTR push_but2_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      buttonTwoStatus=!buttonTwoStatus;
      SWITCH_state[1] = !SWITCH_state[1];
      digitalWrite(18, buttonTwoStatus); 
      last_button_time = button_time;
    }
}
void IRAM_ATTR push_but3_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      buttonThreeStatus=!buttonThreeStatus;
      SWITCH_state[2] = !SWITCH_state[2];
      digitalWrite(19, buttonThreeStatus); 
      last_button_time = button_time;
    }
}
void IRAM_ATTR push_but4_ISR() {
  button_time = millis();
  if (button_time - last_button_time > 500) {
      buttonFourStatus=!buttonFourStatus;
      SWITCH_state[3] = !SWITCH_state[3];
      digitalWrite(21, buttonFourStatus); 
      last_button_time = button_time;
    }
}


void setup() {
  // standard stuff here
  Serial.begin(9600);
  Serial2.begin(115200);
  delay(20);
  tfmP.begin( &Serial2);
  tfmP.sendCommand( SOFT_RESET, 0);
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
  // Initialize Switches
  SWITCH_state[0] = 0; 
  SWITCH_state[1] = 0;

  

 

  // if your web page or XML are large, you may not get a call back from the web page
  // and the ESP will think something has locked up and reboot the ESP
  // not sure I like this feature, actually I kinda hate it
  // disable watch dog timer 0
  disableCore0WDT();

  // maybe disable watch dog timer 1 if needed
  //  disableCore1WDT();

  // just an update to progress
  Serial.println("starting server");

  // if you have this #define USE_INTRANET,  you will connect to your home intranet, again makes debugging easier
#ifdef USE_INTRANET
  WiFi.begin(LOCAL_SSID, LOCAL_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
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
  Serial.print("IP address: "); Serial.println(Actual_IP);
#endif

  printWifiStatus();


  // these calls will handle data coming back from your web page
  // this one is a page request, upon ESP getting / string the web page will be sent
  server.on("/", SendWebsite);

  // upon esp getting /XML string, ESP will build and send the XML, this is how we refresh
  // just parts of the web page
  server.on("/xml", SendXML);

  // upon ESP getting /button??? string, ESP will execute the appropriate function
  server.on("/buttonOne", processButtonOne);
  server.on("/buttonTwo", processButtonTwo);
  //server.on("/buttonThree", processButtonThree);
  //server.on("/buttonFour", processButtonFour);

  // finally begin the server
  server.begin();

}

void loop() {

  // you main loop that measures, processes, runs code, etc.
  // note that handling the "on" strings from the web page are NOT in the loop
  // that processing is in individual functions all managed by the wifi lib

  // in my example here every 50 ms, i measure some analog sensor data (my finger dragging over the pins
  // and process accordingly
  // analog input can be from temperature sensors, light sensors, digital pin sensors, etc.
  if ((millis() - SensorUpdate) >= 50) {
    //Serial.println("Reading Sensors");
    SensorUpdate = millis();
    level_XML = liquidCalib();
    level_XML = map(level_XML,0,50,0,1300);
    temp_XML = ds18b20Calib();
    tds_XML = 200;
    feed_XML = 9;
    press_XML = 1.8;
    triggerCheck();
    
  }
  
  // no matter what you must call this handleClient repeatidly--otherwise the web page
  // will not get instructions to do something
  server.handleClient();

}

// now process button_0 press from the web site. Typical applications are the used on the web client can
// turn on / off a light, a fan, disable something etc
void processButtonOne() {

  buttonOneStatus=!buttonOneStatus;
  digitalWrite(2, buttonOneStatus);
  

  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or let the XML manage
  // sending feeback

  // option 1 -- keep page live but dont send any thing
  // here i don't need to send and immediate status, any status
  // like the illumination status will be send in the main XML page update
  // code
  server.send(200, "text/plain", ""); //Send web page

  // option 2 -- keep page live AND send a status
  // if you want to send feed back immediataly
  // note you must have reading code in the java script
  /*
    if (LED0) {
    server.send(200, "text/plain", "1"); //Send web page
    }
    else {
    server.send(200, "text/plain", "0"); //Send web page
    }
  */

}

// same notion for processing button_1
void processButtonTwo() {

  // just a simple way to toggle a LED on/off. Much better ways to do this
  
  SomeOutput = !SomeOutput;

  digitalWrite(PIN_OUTPUT, SomeOutput);

  // regardless if you want to send stuff back to client or not
  // you must have the send line--as it keeps the page running
  // if you don't want feedback from the MCU--or send all data via XML use this method
  // sending feeback
  server.send(200, "text/plain", ""); //Send web page

  // if you want to send feed back immediataly
  // note you must have proper code in the java script to read this data stream
  /*
    if (some_process) {
    server.send(200, "text/plain", "SUCCESS"); //Send web page
    }
    else {
    server.send(200, "text/plain", "FAIL"); //Send web page
    }
  */
}


// code to send the main web page
// PAGE_MAIN is a large char defined in SuperMon.h
void SendWebsite() {
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/html", PAGE_MAIN);
}

// code to send the main web page
// I avoid string data types at all cost hence all the char mainipulation code
void SendXML() {

  // Serial.println("sending xml");

  strcpy(XML, "<?xml version = '1.0'?>\n<Data>\n");

  // send level_XML
  sprintf(buf, "<G0>%d</G0>\n", level_XML);
  strcat(XML, buf);
  
  // send temp_XML
  sprintf(buf, "<G1>%d</G1>\n", temp_XML);
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
  if (buttonThreeStatus) {
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
     
  strcat(XML, "</Data>\n");
  // wanna see what the XML code looks like?
  // actually print it to the serial monitor and use some text editor to get the size
  // then pad and adjust char XML[2048]; above
  Serial.println(XML);
  
  // you may have to play with this value, big pages need more porcessing time, and hence
  // a longer timeout that 200 ms
  server.send(200, "text/xml", XML);


}

// Function to calculate Feed level in FishFeeder Reservoir from the UART Waterproof distance sensor.
float liquidCalib() {
  tfmP.getData(tfDist);
  level_dist = tfDist;
  delay(10); 
  return level_dist; 
}

// Function to calculate Liquid Temperature  using the DS18B20 Waterproof Temperature Sensor
// (connected to pin 4) using a smoothing technic on a rolling average of last 10 readings.

float ds18b20Calib() {
  sensors.requestTemperatures();
  tempObjecC = sensors.getTempC(tempSensor1);
  //if (tempObjecC < -125) {tempObjecC = previousTemp;}
  //if ((tempObjecC > maxTemp) && (stableTime > 10000)) {maxTemp = tempObjecC;} 
  //if ((tempObjecC > 51) || (tempObjecC < -40)) {tempObjecC = 0; }
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
        /*char ltrChar[7];
        char value2;
        char value3;
        ltrString2 = String(avgDist1);                                            // Convert Liters as float into String 
        if  (!smsWarnOverride) {                                                  // Only be notified once every hour 
            ltrString2.toCharArray(ltrChar, ltrString2.length());                 // Convert Liters as String into char array
            send_webhook(IFTTT_Event1,IFTTT_Key,ltrChar,"value2","value3");
            smsWarnOverride = true;  
            }*/
        }
        if (level_XML < 1099) { statusIndicator1 = 0;}
        if (level_XML > 1099) { statusIndicator1 = 1; }

    // Temperature Level
    if  (temp_XML >= 32) {
        tempConsistHot++;
        tempConsistCold=0;
        }
    if  (temp_XML < 32) {
        tempConsistCold++;;
        tempConsistHot=0;
        }
    if  (tempConsistHot > 10 && !senderOverride) {
        tempConsistHot=0;
       
        /*char value2;
        char value3;
        char tempChar[7];                                                         // Temp as an array
        tempString = String(tempObjecC);                                          // Convert temperature as float into String 
        if  (!smsTempOverride) {                                                  // Only be notified once every hour 
            tempString.toCharArray(tempChar, tempString.length());                // Convert temperature as String into char array
            send_webhook(IFTTT_Event5,IFTTT_Key,tempChar,"value2","value3");
            smsTempOverride = true; 
            }*/
        }
        if (temp_XML < 26) { statusIndicator2 = 1;}
        if (temp_XML >= 28) { statusIndicator2 = 0; }

    
    // TDS Water Quality Warning
    /*if  (tdsValue > 500) {
        tdsConsistHigh++;
        tdsConsistLow=0;
        }
    if  (tdsValue <= 500) {
        tdsConsistLow++;
        tdsConsistHigh=0;
        }
    if  (tdsConsistHigh > 10 && !senderOverride && (stableTime > 60000)) {
        char value1;
        char value2;
        char tdsChar[7];
        tdsString = String(tdsValue);
        if  (!smsTdsOverride) { 
            tdsString.toCharArray(tdsChar, tdsString.length());                // Convert TDS value as String into char array                                                   
            send_webhook(IFTTT_Event4,IFTTT_Key,tdsChar,"value2","value3");
            smsTdsOverride = true;
            }
        }*/ 
        if (tds_XML < 300) { statusIndicator3 = 1;}
        if (tds_XML >= 300) { statusIndicator3 = 0; }
    // FEEDER LEVEL 
    /*if  (distance2 > 10) {
        feedConsistFull++;
        feedConsistLow=0;
        }
    if  (distance2 <= 10) {
        feedConsistLow++;
        feedConsistFull=0;
        }
    if  (feedConsistLow > 10 && !senderOverride && (stableTime > 60000)) {
        feedConsistLow=0;
        char value2;
        char value3;
        char feedChar[7];                                                       
        feedString = String(distance2);                                         // Convert Feed % as float into String 
        if  (!smsFeedOverride) {                                                // Only be notified once every hour 
            feedString.toCharArray(feedChar, feedString.length());              // Convert Feed as String into char array
            //send_webhook(IFTTT_Event3,IFTTT_Key,feedChar,"value2","value3");
            smsFeedOverride = true;   
            } 
        }*/
        if (feed_XML < 10) { statusIndicator4 = 0;}
        if (feed_XML > 10) { statusIndicator4 = 1; }
        
}



void printWifiStatus() {

  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
  // print where to go in a browser:
  Serial.print("Open http://");
  Serial.println(ip);
}
