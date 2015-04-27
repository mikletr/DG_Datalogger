#include <SPI.h>
#include <SD.h>
#include <EEPROM.h>

// Constants
const char* GPS_NEW_DATA_PREFIX = "@";
const char* GPS_OLD_DATA_PREFIX = "#";
const char* ECHO_NEW_DATA_PREFIX = "%";
const char* ECHO_OLD_DATA_PREFIX = "&";

const char BT_RTS_PREFIX = *"!";
const char BT_DSR_PREFIX = *"*";


#define BT_CONNECT_PIN A6

HardwareSerial &SerialPC = Serial;
HardwareSerial &SerialGPS = Serial1;
HardwareSerial &SerialECHO = Serial2;
HardwareSerial &SerialBT = Serial3;

const int GPS_BAUDRATE = 9600;
const int ECHO_BAUDRATE = 9600;
const int BT_BAUDRATE = 9600;
const int PC_BAUDRATE = 9600;

const int BUFFER_LEN = 80;


boolean connectedBT = false;
boolean rtsBT = false; // BT waits data

boolean sdCardInitialized = false;

const int chipSelect = 4;

File dataFile;
boolean dataFileOpened = false;

unsigned long bytesWritten;
unsigned long bytesRead;

int addrBytesWritten = 0;
int addrBytesRead = 16;


String inputStringGPS = "";         // a string to hold incoming data from GPS
boolean stringCompleteGPS = false;  // whether the string from GPS is complete
int inputStringGPSlenght = 0;

String inputStringECHO = "";         // a string to hold incoming data from sonar
boolean stringCompleteECHO = false;  // whether the string from sonar is complete
int inputStringECHOlenght = 0;

String inputStringBT = "";         // a string to hold incoming data from bluetooth
boolean stringCompleteBT = false;  // whether the string from bluetooth is complete

unsigned long stopWatchStartTime;
unsigned long stopWatchResult;

void setup()
{
  inputStringGPS.reserve(BUFFER_LEN);
  inputStringECHO.reserve(BUFFER_LEN);

  connectedBT = false;
  rtsBT = false;

  pinMode(BT_CONNECT_PIN, INPUT); // connectedBT
  
  EEPROM.get(addrBytesWritten, bytesWritten);
  EEPROM.get(addrBytesRead, bytesRead);

  connectedBT = digitalRead(BT_CONNECT_PIN);

  SerialPC.begin(PC_BAUDRATE);
  while (!SerialPC) {;}

  SerialGPS.begin(GPS_BAUDRATE);
  while (!SerialGPS) {;}

  delay(300); //delay between GPS logger and sonar system, if suetable

  SerialECHO.begin(ECHO_BAUDRATE);
  while (!SerialECHO) {;}

  SerialBT.begin(BT_BAUDRATE);
  while (!SerialBT) {;}

  SerialPC.print("Initializing SD card...");
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(SS, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    SerialPC.println("Card failed, or not present");
    
    sdCardInitialized = false;
  }
  else
  {
    SerialPC.println("card initialized.");
    
    sdCardInitialized = true;
    
    // Open up the file we're going to log to!
    dataFile = SD.open("datalog.txt", FILE_WRITE);
    if (! dataFile) {
      SerialPC.println("error opening datalog.txt");
      // Wait forever since we cant write data
      //while (1) ;
    }
    else
    {
      dataFileOpened = true;
    }
  }
}

void loop()
{
  // get byte from GPS UART
  if (SerialGPS.available()) {
    char inChar = (char)SerialGPS.read();
    inputStringGPS += inChar;
    inputStringGPSlenght++;
    if (inChar == '\n') {
      stringCompleteGPS = true;
    }
  }

 if (connectedBT) {
   // BT request data
   if(SerialBT.available())
   {
      char inChar = (char)SerialBT.read();
      if(inChar == BT_RTS_PREFIX)
      {
        rtsBT = true;
      }
   }
   
   if(rtsBT == true)
   {
     // send buffer to BT
   }
   
   
  }

  if (stringCompleteECHO) {

    inputStringECHO = ECHO_NEW_DATA_PREFIX + inputStringECHO;
    SerialPC.println(inputStringECHO);
    dataFile.print(inputStringECHO);
    bytesWritten += inputStringECHOlenght;
    inputStringECHO = "";
    stringCompleteECHO = false;
    inputStringECHOlenght = 0;
  }

  // get byte from sonar UART
  if (SerialECHO.available()) {
    char inChar = (char)SerialECHO.read();
    inputStringECHO += inChar;
    inputStringECHOlenght++;
    if (inChar == '\n') {
      stringCompleteECHO = true;
    }
  }

  // print the string when a newline arrives:
  if (stringCompleteGPS) {

    inputStringGPS = GPS_NEW_DATA_PREFIX + inputStringGPS;
    SerialPC.println(inputStringGPS);
    dataFile.print(inputStringGPS);
    bytesWritten += inputStringGPSlenght;
    dataFile.flush();
    inputStringGPS = "";
    stringCompleteGPS = false;
    inputStringGPSlenght = 0;
  }

  //  dataFile.println(dataString);
  //  dataFile.flush();

  // The following line will 'save' the file to the SD card after every
  // line of data - this will use more power and slow down how much data
  // you can read but it's safer!
  // If you want to speed up the system, remove the call to flush() and it
  // will save the file only every 512 bytes - every time a sector on the
  // SD card is filled with data.
}

String StopStopWatch(String message)
{
  stopWatchResult = millis() - stopWatchStartTime;
  
  return message + stopWatchResult;
}

void StartStopWatch()
{
  stopWatchStartTime = millis();
}
