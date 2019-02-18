#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Timezone.h>
#include <TimeLib.h>
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
byte a[8] = {
  0b01111,
  0b01111,
  0b01111,
  0b01111,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte b[8] = {
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110,
  0b11110
};
byte c[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b11110,
  0b11110,
  0b11110,
  0b11110
};
byte d[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b01111,
  0b01111,
  0b01111,
  0b01111
};
byte e[8] = {
  0b11111,
  0b11111,
  0b11111,
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};
byte f[8] = {
  0b01111,
  0b01111,
  0b01111,
  0b01111,
  0b01111,
  0b01111,
  0b01111,
  0b01111
};
byte g[8] = {
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111,
  0b11111,
  0b11111,
  0b11111
};
const char ssid1[] = "MARFRA1";  //  your network SSID (name)
const char pass1[] = "marfra11";       // your network password
const char ssid2[] = "MARFRA2";  //  your network SSID (name)
const char pass2[] = "marfra11";       // your network password
int i, j, offset, ihour;
// NTP Servers:
IPAddress timeServer(217, 31, 202, 100);//217, 31, 202, 100
//IPAddress timeServer(128, 138, 140, 44); // time.nist.gov NTP server address.timeServer(128, 138, 140, 44)
const char* ntpServerName = "ntp.nic.cz";
const int timeZone = 1;     // Central European Time

WiFiUDP Udp;
unsigned int localPort = 8888;  // local port to listen for UDP packets

void setup()
{
  Serial.begin(115200);
  lcd.begin();
  lcd.backlight();
  lcd.createChar(0, a);
  lcd.createChar(1, b);
  lcd.createChar(2, c);
  lcd.createChar(3, d);
  lcd.createChar(4, e);
  lcd.createChar(5, f);
  lcd.createChar(6, g);
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Connecting to WiFi:");
  lcd.setCursor(0, 1);
  lcd.print(ssid1);
  WiFi.begin(ssid1, pass1);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    lcd.setCursor(i, 2);
    lcd.print(" ");
    if (i < 19) {
      i++;
    } else {
      i = 0;
      WiFi.begin(ssid2, pass2);
    }
    lcd.setCursor(i, 2);
    lcd.print(".");
  }
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("IP:");
  lcd.print(WiFi.localIP());
  lcd.setCursor(0, 1);
  lcd.print("Starting UDP");
  Udp.begin(localPort);
  lcd.setCursor(0, 2);
  lcd.print("Local port: ");
  lcd.print(Udp.localPort());
  lcd.setCursor(0, 3);
  lcd.print("waiting for sync");
  startOta();
  delay(3000);
  lcd.clear();
  setSyncProvider(getNtpTime);
}
time_t prevDisplay = 0; // when the digital clock was displayed

void loop()
{
  ArduinoOTA.handle();
  if (timeStatus() != timeNotSet) {
    if (now() != prevDisplay) { //update the display only if time has changed
      prevDisplay = now();

      if (int(year()) == 2018) {
        if ( int(month()) >= 3 && int(day()) >= 25 && int(hour()) >= 2) {
          offset = 1;
        }
        if ( int(month()) >= 10 && int(day()) >= 28 && int(hour()) >= 2) {
          offset = 0;
        }
      }
      if (int(year()) == 2019) {
        if ( int(month()) >= 3 && int(day()) >= 31 && int(hour()) >= 2) {
          offset = 1;
        }
        if ( int(month()) >= 10 && int(day()) >= 27 && int(hour()) >= 2) {
          offset = 0;
        }
      }
      if (int(year()) == 2020) {
        if ( int(month()) >= 3 && int(day()) >= 29 && int(hour()) >= 2) {
          offset = 1;
        }
        if ( int(month()) >= 10 && int(day()) >= 25 && int(hour()) >= 2) {
          offset = 0;
        }
      }
      if (int(year()) == 2022) {
        if ( int(month()) >= 3 && int(day()) >= 27 && int(hour()) >= 2) {
          offset = 1;
        }
        if ( int(month()) >= 10 && int(day()) >= 30 && int(hour()) >= 2) {
          offset = 0;
        }
      }
      ihour = int(hour()) + offset;
      if (int(ihour) == 24)
        ihour = 0;
      digitalClockDisplay();
    }
  }
}// end loop

void digitalClockDisplay() {
  lcd.setCursor(0, 3);
  printDigits(ihour);
  lcd.print(":");
  printDigits(minute());
  lcd.print(":");
  printDigits(second());
  lcd.setCursor(8, 3);
  lcd.print("  ");
  printDigits(day());
  lcd.print(".");
  printDigits(month());
  lcd.print(".");
  lcd.print(year());
  lcd.setCursor(19, 2);
  if (offset == 1) {
    lcd.print("L");
  } else {
    lcd.print(" ");
  }
  Serial.print(ihour);
  Serial.print(":");
  Serial.print(minute());
  Serial.print(":");
  Serial.print(second());
  Serial.print("  ");
  Serial.print(day());
  Serial.print(".");
  Serial.print(month());
  Serial.print(".");
  Serial.println(year());

  if (second() % 20 || second() % 21 || second() % 22 ) {
    select (int(int(ihour) / 10), 1);
    select ((int(ihour)) - int((int(ihour)) / 10) * 10, 5);
    select (int(minute() / 10), 11);
    select (minute() - int(minute() / 10) * 10, 15);
    if (second() % 2) {
      lcd.setCursor(9, 0);
      lcd.write(6);
      lcd.setCursor(9, 1);
      lcd.write(6);
    } else {
      lcd.setCursor(9, 0);
      lcd.print(" ");
      lcd.setCursor(9, 1);
      lcd.print(" ");
    }
  } else {
    select (int(int(day()) / 10), 1);
    select ((int(day())) - int((int(day())) / 10) * 10, 5);
    select (int(month() / 10), 11);
    select (month() - int(month() / 10) * 10, 15);
    if (second() % 2) {
      lcd.setCursor(9, 2);
      lcd.write(6);
      lcd.setCursor(19, 2);
      lcd.write(6);
    } else {
      lcd.setCursor(9, 2);
      lcd.print(" ");
      lcd.setCursor(19, 2);
      lcd.print(" ");
    }
  }
}
void printDigits(int digits) {
  if (digits < 10)
    lcd.print('0');
  lcd.print(digits);
}
void select(int cas, int xxx ) {
  switch (cas) {
    case 0:
      nula(xxx);
      break;
    case 1:
      jedna(xxx);
      break;
    case 2:
      dve(xxx);
      break;
    case 3:
      tri(xxx);
      break;
    case 4:
      ctyri(xxx);
      break;
    case 5:
      pet(xxx);
      break;
    case 6:
      sest(xxx);
      break;
    case 7:
      sedum(xxx);
      break;
    case 8:
      osum(xxx);
      break;
    case 9:
      devet(xxx);
      break;
  }
}

/*-------- NTP code ----------*/
const int NTP_PACKET_SIZE = 48; // NTP time is in the first 48 bytes of message
byte packetBuffer[NTP_PACKET_SIZE]; //buffer to hold incoming & outgoing packets
time_t getNtpTime()
{
  while (Udp.parsePacket() > 0) ; // discard any previously received packets
  sendNTPpacket(timeServer);
  lcd.setCursor(18, 0);
  lcd.print("Tx");//Transmit NTP Request
  Serial.print("Tx");//Transmit NTP Request

  uint32_t beginWait = millis();
  while (millis() - beginWait < 1500) {
    int size = Udp.parsePacket();
    if (size >= NTP_PACKET_SIZE) {
      lcd.setCursor(18, 0);
      lcd.print("Rx");//Receive NTP Response
      Serial.print("Rx");//Receive NTP Response
      Udp.read(packetBuffer, NTP_PACKET_SIZE);  // read packet into the buffer
      unsigned long secsSince1900;
      // convert four bytes starting at location 40 to a long integer
      secsSince1900 =  (unsigned long)packetBuffer[40] << 24;
      secsSince1900 |= (unsigned long)packetBuffer[41] << 16;
      secsSince1900 |= (unsigned long)packetBuffer[42] << 8;
      secsSince1900 |= (unsigned long)packetBuffer[43];
      const unsigned long seventyYears = 2208988800UL;
      unsigned long epoch = secsSince1900 - seventyYears;
      TimeChangeRule *tcr;
      time_t utc;
      utc = epoch;
      //         printTime(CE.toLocal(utc, &tcr), tcr -> abbrev, "Bratislava");
      return secsSince1900 - 2208988800UL + timeZone * SECS_PER_HOUR;
      return seventyYears + utc;
    }
  }
  lcd.setCursor(0, 3);
  lcd.print("No NTP Response :-(");
  Serial.print("No NTP Response :-(");
  lcd.setCursor(18, 0);
  lcd.print("No");
  return 0; // return 0 if unable to get the time
}
// send an NTP request to the time server at the given address
void sendNTPpacket(IPAddress &address)
{
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  Udp.beginPacket(address, 123); //NTP requests are to port 123
  Udp.write(packetBuffer, NTP_PACKET_SIZE);
  Udp.endPacket();
}
