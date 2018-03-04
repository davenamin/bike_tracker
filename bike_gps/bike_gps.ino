/**
   Daven Amin, 01/27/18
   sketch for the arduino pro-ish board on the naza gps clone

   this sketch sends a few UBX messages on load,
   UBX-LOG-CREATE to create a maximum size circular log on the uBlox
   UBX-CFG-LOGFILTER to enable the recording of gps fixes

   several UBX-CFG-MSG commands to disable NMEA GGA, GLL, GSA, GSV, RMC, VTG msgs

   it then acts as a serial passthrough at 9600 baud, but after a small period of
   inactivity it passes on mock NMEA GPTXT strings containing information
   from the onboard compass and ATMega's clock (using Arduino's millis() function)
*/
#include <math.h>
#include <SoftwareSerial.h>
#include <Wire.h>

// this is the serial connection to the uBlox
SoftwareSerial mySerial(8, 9); // RX, TX

// this is the I2C address for the HMC5883L compass
#define COMPADDRESS 0x1E

// used for compass heading calc
#define RAD_TO_DEG 57.29578

void setup() {

  // give the GPS and compass a chance to get going
  delay(500);

  // Open I2C communications
  Wire.begin();

  // configure compass (might be a HMC5983?)
  Wire.beginTransmission(COMPADDRESS);
  Wire.write(0x00); // config register A
  Wire.write(0xF0); // temperature adjusted 8avg sample, 15hz output rate
  Wire.endTransmission();

  Wire.beginTransmission(COMPADDRESS);
  Wire.write(0x01); // config register B
  Wire.write(0x20); // set gain to default
  Wire.endTransmission();

  Wire.beginTransmission(COMPADDRESS);
  Wire.write(0x02); // mode register
  Wire.write(0x01); // single measurement mode
  Wire.endTransmission();

  // wait for first measurement
  delay(70);

  // Open serial communications
  Serial.begin(9600);
  mySerial.begin(9600);


  // values from the hex display of the uCenter messages view
  uint8_t ubxLogCreate[] = {0xB5, 0x62, 0x21, 0x07, 0x08, 0x00, // header and length
                            0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // create max size circular log
                            0x31, 0x30 // checksum
                           };
  int logCreateLen = 16;
  uint8_t ubxCfgLogfilter[] = {0xB5, 0x62, 0x06, 0x47, 0x0C, 0x00, // header and length
                               0x01, 0x01, // version 1, enable recording
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // unset
                               0x5B, 0x48 //checksum
                              };
  int logStartLen = 20;

  sendMessage(ubxLogCreate, logCreateLen);
  sendMessage(ubxCfgLogfilter, logStartLen);


  // disable GGA
  uint8_t ubxCfgMsgNMEA[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, // header and length
                             0xF0, 0x00, // msg type
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x01, // send rate
                             0x00, 0x24 // checksum
                            };
  int msgCfgLen = 16;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);

  int msgIx = 7;
  int checksumIx = 14;

  // disable GLL
  ubxCfgMsgNMEA[msgIx] = 0x01;
  ubxCfgMsgNMEA[checksumIx] = 0x01;
  ubxCfgMsgNMEA[checksumIx + 1] = 0x2B;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);

  // disable GSA
  ubxCfgMsgNMEA[msgIx] = 0x02;
  ubxCfgMsgNMEA[checksumIx] = 0x02;
  ubxCfgMsgNMEA[checksumIx + 1] = 0x32;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);

  // disable GSV
  ubxCfgMsgNMEA[msgIx] = 0x03;
  ubxCfgMsgNMEA[checksumIx] = 0x03;
  ubxCfgMsgNMEA[checksumIx + 1] = 0x39;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);

  // disable RMC
  ubxCfgMsgNMEA[msgIx] = 0x04;
  ubxCfgMsgNMEA[checksumIx] = 0x04;
  ubxCfgMsgNMEA[checksumIx + 1] = 0x40;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);

  // disable VTG
  ubxCfgMsgNMEA[msgIx] = 0x05;
  ubxCfgMsgNMEA[checksumIx] = 0x05;
  ubxCfgMsgNMEA[checksumIx + 1] = 0x47;
  sendMessage(ubxCfgMsgNMEA, msgCfgLen);
}

void sendMessage(uint8_t payload[], int len)
{
  // clear the mySerial buffer?
  while (mySerial.available() > 0) {
    Serial.write(mySerial.read());
  }

  for (int ii = 0; ii < len; ii++) {
    mySerial.write(payload[ii]);
  }
  mySerial.println();
  mySerial.flush();
  Serial.write(payload, len);
  Serial.println();
}

unsigned long lastReadWrite = 0;
unsigned const long quietPeriod = 2000;
void loop() { // run over and over

  // passthrough
  if (mySerial.available() > 0) {
    Serial.write(mySerial.read());
    lastReadWrite = millis();
  }
  if (Serial.available() > 0) {
    mySerial.write(Serial.read());
    lastReadWrite = millis();
  }


  // write out compass messages if not doing serial passthrough
  unsigned long diff = abs(millis() - lastReadWrite);
  if (diff > quietPeriod) {
    // get a new compass measurement
    Wire.beginTransmission(COMPADDRESS);
    Wire.write(0x02); // mode register
    Wire.write(0x01); // single measurement mode
    Wire.endTransmission();

    delay(6); // delay 6ms to allow compass to measure

    // read the compass (already pointing at correct register)
    short x, y, z; // triple axis data

    //Read data from each axis, 2 registers per axis
    Wire.requestFrom(COMPADDRESS, 6);
    if (6 <= Wire.available()) {
      x = Wire.read() << 8; //X msb
      x |= Wire.read(); //X lsb
      z = Wire.read() << 8; //Z msb
      z |= Wire.read(); //Z lsb
      y = Wire.read() << 8; //Y msb
      y |= Wire.read(); //Y lsb
    }

    // also want temperature
    Wire.beginTransmission(COMPADDRESS);
    Wire.write(0x31); // temp register MSB
    Wire.endTransmission();

    short temp;

    //Read data from 2 temp registers
    Wire.requestFrom(COMPADDRESS, 2);
    if (2 <= Wire.available()) {
      temp = Wire.read() << 8; //temp msb
      temp |= Wire.read(); //temp lsb
    }

    float celsius = temp / 128.0f + 25;

    // compass heading
    // compass is mounted upside down...
    // -y points forward, x points to the right
    float f_heading = atan2((float)x,(float)y) * RAD_TO_DEG;
    int heading = round(f_heading);

    // Print out values of each axis as fake GPTXT msg
    String headtext = "GPTXT,01,07,millis: ";
    headtext += String(millis());
    headtext += String("  heading: ");
    headtext += String(heading);
    headtext += String("  temp: ");
    headtext += String(celsius);
    headtext += String("  x: ");
    headtext += String(x);
    headtext += String("  y: ");
    headtext += String(y);
    headtext += String("  z: ");
    headtext += String(z);
    headtext += String(" ");
    byte checksum = headtext[0];
    for (int ii = 1; ii < headtext.length(); ii++) {
      checksum = checksum ^ headtext[ii];
    }

    Serial.print("$");
    Serial.print(headtext);
    Serial.print("*");
    Serial.println(checksum, HEX);

    // allow passthrough for the next quiet period
    lastReadWrite = millis();
  }
}

