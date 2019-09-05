/*
ESP_main.cpp - Main arduino source code that hosts a web app (from index_html.h) 
and then moves stepper motors to draw a picture
by Max Leblang 2019
*/
#include <Arduino.h>
#include "WiFi.h"
#include "index_html.h"
#include <Stepper.h>
#include <LiquidCrystal_I2C.h>
 
#define STEPS 200//steps on the stepper
#define APPWIDTH 950
#define APPHEIGHT 800
#define STEPPERMAX_X 6050
#define STEPPERMAX_Y 10000
#define X_INTERRUPT 23
 
 
//objects for LCD
//LiquidCrystal_I2C lcd(0x3F,16,2);
// sometimes 0x27 works when 0x3F doesn't
// that's the I2C address of the backpack 
//objects for Wifi
WiFiServer server(80);//web server port to 80
String header;//variable for HTTP requests
 
//Network credentials
const char* ssid     = "CHS Guest Wireless";
const char* password = "";
//arduino pins
const int resetPin = 2;
//stepper motors
Stepper stepperX(STEPS, 25, 26, 27, 14);
Stepper stepperY(STEPS, 17, 5, 18, 19);
int stepperPos[2] = {0,0};//current position of stepper in [x,y]
int currentPosX = 0;
int currentPosY = 0;
//calibration pins
bool Xcalibrated = false;
int xSideInterruptState = 0;
 
void Xcalibrate(){//interrupting when the stepper gerts to 0 and completes circuit
  xSideInterruptState = digitalRead(X_INTERRUPT);
  if(xSideInterruptState == HIGH){
    Serial.println("Calibrated");
    Xcalibrated = true;
  }
}
 
void setup() {
  Serial.begin(9600);
  //reset setup
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, HIGH);//making sure ESP doesn't auto reset
  //LCD setup
  // lcd.init();
  // lcd.backlight();
  // lcd.setCursor(0,0);
  //stepper setup for 30 rpm
  stepperX.setSpeed(200);
  stepperY.setSpeed(200);
  //Calibration; setting stepper to 0
 
  attachInterrupt(digitalPinToInterrupt(X_INTERRUPT), Xcalibrate, CHANGE);
  while(Xcalibrated == false){
    stepperX.step(-1);
  }
  detachInterrupt(digitalPinToInterrupt(X_INTERRUPT));
 
  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  //lcd.print("Connecting");
  //lcd.setCursor(0,1);
  Serial.println(ssid);
  //lcd.print(ssid);
  WiFi.begin(ssid, password);//connecting to Wifi
  int resetCounter = 0;//variable to count how long connecting takes
  while (WiFi.status() != WL_CONNECTED) {//connecting
    delay(500);
    Serial.print(".");
    resetCounter++;
    //resetting ESP if it takes to long to connect
    if(resetCounter > 6){
      digitalWrite(resetPin, LOW);
      Serial.print("Resetting");
      Serial.print(resetCounter);
    }
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  // lcd.clear();
  // lcd.setCursor(0,0);
  // lcd.print("IP is");
  // lcd.setCursor(0,1);
  // lcd.print(WiFi.localIP());
  server.begin();
}
 
void loop(){
  WiFiClient client = server.available();// Listen for incoming clients
 
  if (client) {// If a new client connects,
    client.setTimeout(8);
    Serial.println("New Client.");// print a message out in the serial port
    String currentLine = "";// make a String to hold incoming data from the client
    while (client.connected()){// loop while the client's connected
      if (client.available()){// if there's bytes to read from the client,
        char c = client.read();// read a byte, then
        Serial.write(c);// print it out the serial monitor
        header += c;//makes header
 
        if (c == '\n') {// if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            //printing out html page
            client.println(htmlPage);
            client.println("");
 
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          }
          else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
 
        }
        else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
 
      }
 
    }
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    //put arduino code here
 
    //variables to build substring;
    String header_substring = " ";
    int pos1 = 0;
    int pos2 = 0;
    //making substring of just numbers
    if(header.indexOf("GET /?values=")>=0){
      //making substring of only the values
      pos1 = header.indexOf('X');
      pos2 = header.lastIndexOf('!');
      header_substring = header.substring(pos1+1, pos2-1);
      Serial.println("HEADER SUBSTRING----");
      Serial.println(header_substring);
    }
    //iterating through the header
    //variables for processing HTTP request
    int header_size = header.substring(header.indexOf('(')+1,header.indexOf(')')).toInt();//finding # of positions from (...) in header
    int headerX[header_size] = {};
    int headerY[header_size] = {};
    long int arrayPos = 0;
    bool addTo = 0;//adding to X if 0 and Y if 1
    String addNumber = "";//full number from the HTTP request
    char headerPos = ' ';
 
    Serial.print("SIZE:");
    Serial.println(header_size);
    //printing to lcd
 
    // if(header.indexOf("/?values") > 0){//checking if header has data
    //   lcd.clear();//resetting LCD for arduino functions
    //   lcd.setCursor(0,0);
    //   lcd.print("Data recieved!");
    //   lcd.setCursor(0,1);
    //   lcd.print(header_size);
    //   lcd.print(" points");
    // }
 
    //parsing through header_substring
    for(int i = 0; i < header_substring.length(); i++){
      headerPos = header_substring.charAt(i);
      //building header arrays
      switch(headerPos){
        case ','://going to next number
          //Serial.println(",");
          if(addTo == 0){//adding the full number to X & mapping position
            headerX[arrayPos] = round(map(addNumber.toInt(), 0, APPWIDTH, 0, STEPPERMAX_X));
          }
          else{//adding the full number to Y & mapping position
            headerY[arrayPos] = round(map(addNumber.toInt(), 0, APPHEIGHT, 0, STEPPERMAX_Y));
          }
          arrayPos++;//move along in array
          addNumber = "";//resetting add number
          break;
 
        case 'Y'://switching to Y array
          //Serial.println("Y");
          addTo = 1;
          arrayPos = 0;
          break;
 
        default://building the full number char by char
          //Serial.print(headerPos);
          addNumber += String(headerPos);
          break;
      }
 
    }
    Serial.println("DONE!!");
    //drawing image
    for(int i = 0; i<header_size; i++){//looping through position arrays
      if(headerX[i] == -6){
        //lifting up arm
        Serial.println("lifing up");
      }
      else{
        //moving stepper
        //figuring where to go
        int stepAmmount = headerX[i]-currentPosX;//finding how far to move
        int stepDir = (stepAmmount < 0)? -1: 1;//whether to move forward or backward
        Serial.println(headerX[i]);
        //looping through each step
        for(int i = 0; i <= abs(stepAmmount); i++){
          stepperX.step(1*stepDir);//multiply by stepDir for correct direction
        }
        currentPosX = headerX[i];//setting currentPosX to new position
        delay(5);
      }
 
    }
 
    //restting all arrays  
    memset(headerX, 0, sizeof(headerX));
    memset(headerY, 0, sizeof(headerY));
  }
// Clear the header variable
header = "";
//resetting addTo to add to X next time
}
