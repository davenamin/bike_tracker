/**
   sketch for the bike computer.
   Daven Amin, 01/21/2018

   uses code from:
   https://www.arduino.cc/en/Tutorial/ConnectWithWPA

   uses Adafruit's SSD1306/GFX libraries:
   https://github.com/adafruit/Adafruit_SSD1306
   https://github.com/adafruit/Adafruit-GFX-Library

   uses https://github.com/bblanchon/ArduinoJson

   the gist of this sketch is:
   on setup, if we can connect to the known WiFi SSID,
   push everything from the ublox module to a known server

   on loop, calculate and display stats from GPS output
*/

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SoftwareSerial.h>

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include <ArduinoJson.h>

// defines for WIFI_SSID, WIFI_PASS
#include "keys.h"

// taken from https://learn.adafruit.com/adafruit-oled-featherwing/usage
// begin taken code
Adafruit_SSD1306 display = Adafruit_SSD1306();

#if defined(ESP8266)
#define BUTTON_A 0
#define BUTTON_B 16
#define BUTTON_C 2
#define LED      0
#elif defined(ESP32)
#define BUTTON_A 15
#define BUTTON_B 32
#define BUTTON_C 14
#define LED      13
#elif defined(ARDUINO_STM32F2_FEATHER)
#define BUTTON_A PA15
#define BUTTON_B PC7
#define BUTTON_C PC5
#define LED PB5
#elif defined(TEENSYDUINO)
#define BUTTON_A 4
#define BUTTON_B 3
#define BUTTON_C 8
#define LED 13
#elif defined(ARDUINO_FEATHER52)
#define BUTTON_A 31
#define BUTTON_B 30
#define BUTTON_C 27
#define LED 17
#else // 32u4, M0, and 328p
#define BUTTON_A 9
#define BUTTON_B 6
#define BUTTON_C 5
#define LED      13
#endif

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif
// end taken code

// used to communicate with GPS module
SoftwareSerial swSer(14, 12, false, 256);

void setup() {
  Serial.begin(9600);
  swSer.begin(9600);
  WiFi.mode(WIFI_STA);
  Serial.setDebugOutput(false);//true);


  // taken and modified from https://learn.adafruit.com/adafruit-oled-featherwing/usage
  pinMode(BUTTON_A, INPUT_PULLUP);
  pinMode(BUTTON_B, INPUT_PULLUP);
  pinMode(BUTTON_C, INPUT_PULLUP);


  Serial.println("OLED FeatherWing test");
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  // init done
  Serial.println("OLED begun");

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.
  display.display();
  delay(1000);

  // Clear the buffer.
  display.clearDisplay();
  display.display();

  // text display
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);

  // attempt to connect to Wifi network:
  if ( WiFi.status() != WL_CONNECTED) {
    display.print("Attempting to connect to WPA SSID: ");
    display.println(WIFI_SSID);
    display.display(); // actually display all of the above
    // Connect to WPA/WPA2 network:
    WiFi.begin(WIFI_SSID, WIFI_PASS);

    // wait 5 seconds for connection:
    delay(5000);
  }
  display.clearDisplay();
  WiFi.printDiag(display);
  display.display();
}

bool clearFlag = true;
void loop() {
  if (swSer.available()) {
    int val = swSer.read();
    Serial.write(val);

    if (clearFlag)
    {
      display.clearDisplay();
      clearFlag = false;
    }
    char cval = char(val);
    display.print(cval);
    if (cval == '\n') {
      display.display();
      display.setCursor(0, 0);
      clearFlag = true;
    }
  }
  if (Serial.available()) {
    int val = Serial.read();
    swSer.write(val);

    if (clearFlag)
    {
      display.clearDisplay();
      clearFlag = false;
    }
    char cval = char(val);
    display.print(cval);
    if (cval == '\n') {
      display.display();
      display.setCursor(0, 0);
      clearFlag = true;
    }

  }
  yield();
}

/*
  const char* time_service = "time.nist.gov";
  const int time_port = 13;
  void updateTime() {
  WiFiClient client;
  TimeSpan tz_offset = TimeSpan(0, 5, 0, 0);
  // set the initial time if we can
  if (client.connect(time_service, time_port))
  {
    String datetime = client.readString();
    Serial.println("NIST time follows:");
    Serial.println(datetime);
    Serial.println("Parsing NIST time");
    if (datetime.length() >= 25)
    {
      // NIST format: JJJJJ YR-MO-DA HH:MM:SS TT L H msADV UTC(NIST) OTM
      // where YR-MO-DA HH:MM:SS are parsed below
      // looks like the datetime string has added linefeeds on the start and end...
      String year = datetime.substring(7, 9);
      String month = datetime.substring(10, 12);
      String day = datetime.substring(13, 15);
      String hour = datetime.substring(16, 18);
      String minute = datetime.substring(19, 21);
      String second = datetime.substring(22, 24);
      Serial.print("year: " + year);
      Serial.print("  month: " + month);
      Serial.print("  day: " + day);
      Serial.print("time: " + hour + ":" + minute + ":" + second);
      Serial.println();

      rtc.adjust(DateTime(year.toInt(), month.toInt(), day.toInt(),
                          hour.toInt(), minute.toInt(), second.toInt()) - tz_offset);
      Serial.println("Set time!");
    }
  }
  }
*/

