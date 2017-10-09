#include <GPSport.h>
#include <SoftwareSerial.h>
#include <NMEAGPS.h>
#define GSM_PORT softSerial
#define ON LOW
#define OFF HIGH
SoftwareSerial softSerial(6,7); // RX, TX
NMEAGPS  gps; // Parshing Karakter dari GPS
gps_fix  fix; // Menampilkan data jika GPS Lock
#define gpsPort Serial

String bacaSMS = "";
String msg = "";
bool siap = 0;

unsigned int errorCounter;

const int Relay1 = 8;
const int Relay2 = 9;
const int Relay3 = 10;
const int Relay4 = 11;

int StatRelay1;
int StatRelay2;
int StatRelay3;
int StatRelay4;

void setup()
{
  pinMode (Relay1 , OUTPUT);
  digitalWrite (Relay1, HIGH);
  pinMode (Relay2 , OUTPUT);
  digitalWrite (Relay2, HIGH);
  pinMode (Relay3 , OUTPUT);
  digitalWrite (Relay3, HIGH);
  pinMode (Relay4 , OUTPUT);
  digitalWrite (Relay4, HIGH);
  StatRelay1 = OFF;
  StatRelay2 = OFF;
  StatRelay3 = OFF;
  StatRelay4 = OFF;

  DEBUG_PORT.begin(9600);
  GSM_PORT.begin(9600);
  while (!Serial);

  gpsPort.begin(9600);

  delay(30000);
  GSM_PORT.println("AT+CMGF=1");
  delay(1000);
  GSM_PORT.println("AT+CNMI=2,2,0,0,0");
  delay(1000);
  //  GSM_PORT.println("AT+CNMI=1,2,0,0,0");
  //  delay(1000);
  GSM_PORT.println("AT+CIPMUX=0");
  delay(1000);
  GSM_PORT.println("AT+CGATT=1");
  delay(1000);
  GSM_PORT.println("AT+CSTT=\"internet\",\"\",\"\"");
  delay(1000);
  GSM_PORT.println("AT+CIICR");
  delay(2000);
  GSM_PORT.println("AT+CIFSR");
  delay(1000);
  GSM_PORT.println("AT+CIPSTART=\"TCP\",\"domainku.com\",5109");
  delay(2000);
}

uint8_t fixCount = 0;
void loop() {
  digitalWrite(Relay1, StatRelay1);
  digitalWrite(Relay2, StatRelay2);
  digitalWrite(Relay3, StatRelay3);
  digitalWrite(Relay4, StatRelay4);

  // GPS UBLOX M8N
  if (gps.available(gpsPort)) {
    while (gps.available(gpsPort) > 0) {
      fix = gps.read();
      //    fixCount++;
      //    if (fixCount >= 5) { // interval kirim data ke server 5 detik
      //      fixCount = 0;
      if (fix.valid.location) {
        GSM_PORT.println("AT+CIPSEND");
        GSM_PORT.print("FOLLOWIT,123456789012345,");
        char tanggalDanJam[12];
        sprintf(tanggalDanJam, "%02d%02d%02d%02d%02d", fix.dateTime.month, fix.dateTime.date, fix.dateTime.hours, fix.dateTime.minutes, fix.dateTime.seconds);
        GSM_PORT.print(fix.dateTime.year);
        GSM_PORT.print(tanggalDanJam);
        GSM_PORT.print(",");
        GSM_PORT.print(fix.latitude(), 6);
        GSM_PORT.print(",");
        GSM_PORT.print(fix.longitude(), 6);
        GSM_PORT.print(",");
        int i;
        float kecepatan;
        kecepatan = fix.speed_kph();
        i = (int) kecepatan;
        GSM_PORT.print(i);
        GSM_PORT.print(",");
        GSM_PORT.print("0");
        GSM_PORT.print(",");
        GSM_PORT.print(fix.satellites);
        GSM_PORT.print(",");
        int j;
        float altit;
        altit = fix.altitude();
        j = (int) altit;
        GSM_PORT.print(j);
        GSM_PORT.print(",F,");
        delay(1000);
        GSM_PORT.write(0x1A);
        delay(1000);
      }
    }
  }
  // AKHIR GPS UBLOX M8N

  // SMS RELAY
  if (GSM_PORT.available()) {
    while (GSM_PORT.available() > 0) {
      bacaSMS += (char)GSM_PORT.read();
      if (bacaSMS.indexOf("+6281312357771") >= 0) {
        if (bacaSMS.indexOf("hidup") >= 0) {
          StatRelay1 = ON;
          digitalWrite(Relay1, StatRelay1);
          delay(7000);
          StatRelay2 = ON;
          digitalWrite(Relay2, StatRelay2);
          delay(500);
          StatRelay3 = ON;
          digitalWrite(Relay3, StatRelay3);
          delay(3000);
          StatRelay2 = OFF;
          StatRelay3 = OFF;
          siap = 1;
          errorCounter = 0;
          bacaSMS.remove(0);
        }
        else if (bacaSMS.indexOf("mati") >= 0) {
          StatRelay1 = OFF;
          siap = 0;
          errorCounter = 0;
          bacaSMS.remove(0);
        }
        else if (bacaSMS.indexOf("posisi") >= 0) {
          GSM_PORT.println("AT+CMGS=\"+6281312357771\"");
          delay(1000);
          // latitude S hilangkan -, longtitud E
          GSM_PORT.print("http://maps.google.com/maps?q=");
          String latS = String(fix.latitude(), 6);
          latS.replace("-", "S");
          GSM_PORT.print(latS);
          GSM_PORT.print(",E");
          GSM_PORT.println(fix.longitude(), 6);
          delay(1000);
          GSM_PORT.write(0x1A);
          siap = 0;
          errorCounter = 0;
          bacaSMS.remove(0);
        }
      }
    }
  }
  // AKHIR SMS RELAY
}
