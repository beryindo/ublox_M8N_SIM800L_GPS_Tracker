#include <gprs.h>
#include <GPSport.h>
#include <SoftwareSerial.h> 

#define USE_SCREEN
#include <NMEAGPS.h>

#ifdef USE_SCREEN
  #include <SPI.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_PCD8544.h>
  Adafruit_PCD8544 display = Adafruit_PCD8544(5, 4, 3);
#endif // USE_SCREEN

#define GSM_PORT softSerial

SoftwareSerial softSerial(6, 7); // RX, TX
#include "GSM.h"

NMEAGPS  gps; // This parses the GPS characters
gps_fix  fix; // This holds on to the latest values
GPRS gprs;

//--------------------------
// CHECK CONFIGURATION

#if !defined(GPS_FIX_TIME) | !defined(GPS_FIX_DATE)
  #error You must define GPS_FIX_TIME and DATE in GPSfix_cfg.h!
#endif

#if !defined(NMEAGPS_PARSE_RMC) & !defined(NMEAGPS_PARSE_ZDA)
  #error You must define NMEAGPS_PARSE_RMC or ZDA in NMEAGPS_cfg.h!
#endif

//#include <GPSport.h>
#define gpsPort Serial

//--------------------------
// Set these values to the offset of your timezone from GMT

static const int32_t          zone_hours   = +7L; // EST
static const int32_t          zone_minutes =  0L; // usually zero
static const NeoGPS::clock_t  zone_offset  =
                                zone_hours   * NeoGPS::SECONDS_PER_HOUR +
                                zone_minutes * NeoGPS::SECONDS_PER_MINUTE;

void adjustTime( NeoGPS::time_t & dt )
{
  NeoGPS::clock_t seconds = dt; // convert date/time structure to seconds

  //  First, offset from UTC to the local timezone
  seconds += zone_offset;

  #ifdef CALCULATE_DST
    //  Then add an hour if DST is in effect
    if ((springForward <= seconds) && (seconds < fallBack))
      seconds += NeoGPS::SECONDS_PER_HOUR;
  #endif

  dt = seconds; // convert seconds back to a date/time structure

} // adjustTime

                                
void setup()
{
  DEBUG_PORT.begin(9600);
  GSM_PORT.begin(9600);
  while (!Serial);

  gpsPort.begin(9600);
#ifdef USE_SCREEN
  display.begin();
  display.setContrast(90); // you may need to adjust this for your screen

  display.setTextSize(1);
  display.setTextColor(BLACK);
  display.setCursor(0,0);

  display.clearDisplay();
  display.println("Init GPS...");  
  display.display();
#endif // USE_SCREEN

#ifdef USE_SCREEN
  display.println("Nunggu GSM...");  
  display.display();
#endif // USE_SCREEN
  // wait ten seconds for GSM module to connect to mobile network
  delay(10000);

  sendGSM("AT+SAPBR=3,1,\"APN\",\"internet\"");  // change this for your cell provider
  delay(100);
  sendGSM("AT+SAPBR=1,1",3000);
  delay(100);
  sendGSM("AT+HTTPINIT");
  delay(100);
  sendGSM("AT+HTTPPARA=\"CID\",1");
  delay(100);
  sendGSM("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\"");   
  
#ifdef USE_SCREEN  
  display.println("Check GPS...");
  display.display();
#endif // USE_SCREEN

#ifdef USE_SCREEN
  display.println("Selesai.");
  display.display();
#endif // USE_SCREEN
}

//--------------------------
void jam()
{
    // Display the local time

    if (fix.valid.time && fix.valid.date) {
      adjustTime( fix.dateTime );

      DEBUG_PORT << fix.dateTime;
    }
}

void konekGSM(unsigned long now) {
  while(GSM_PORT.available()) {
    lastActionTime = now;
    parseATText(GSM_PORT.read());
  }
}

char* spinner = "/-\\|";
byte spinnerPos = 0;
uint8_t fixCount = 0;
void loop()
{
  unsigned long now = millis();
  static unsigned long lastScreenUpdate = 0;        
  if (gprs.getIPAddress()=="0.0.0.0")
  {
    sendGSM("AT+SAPBR=1,1",3000);
  }
  while (gps.available( gpsPort )) {
    fix = gps.read();
    fixCount++;
    if (fixCount >= 5) {
      fixCount = 0;
    
//    if ( actionState == AS_IDLE ) {
    if (fix.longitude() > 0 ) {
      GSM_PORT.print("AT+HTTPPARA=\"URL\",\"http://gpsindo.web.id:5055/?id=masukannohpdisini");
      GSM_PORT.print( F("&lat=") );
        GSM_PORT.print( fix.latitude(), 7 );
      GSM_PORT.print( F("&lon=") );
        GSM_PORT.print( fix.longitude(), 7 );
      GSM_PORT.print( F("&timestamp=") );
        GSM_PORT.print(fix.dateTime+946710266);
//    jam();    
      GSM_PORT.print( F("&hdop=") );
        GSM_PORT.print( fix.hdop );
      GSM_PORT.print( F("&altitude=") );
        GSM_PORT.print( fix.altitude() ); 
      GSM_PORT.print( F("&speed=") );
        GSM_PORT.print( fix.speed_kph() );      
      GSM_PORT.println("\"");
      sendGSM("AT+HTTPACTION=1");
      sendGSM("AT+HTTPPARA=\"CONTENT\",\"application/x-www-form-urlencoded\""); 

    }
  }

  }
#ifdef USE_SCREEN
  if ( (now - lastScreenUpdate) > 500 ) {
    display.clearDisplay();
    display.print(spinner[spinnerPos]);
    spinnerPos = (spinnerPos + 1) % 4;     
      display.print(" Sat:");
      display.println(fix.satellites);
//      display.print("IP");
//      display.println(gprs.getIPAddress()); 
//        konekGSM(now);
//      display.print(" (");      
//      display.print(httpResult);
//      display.println(")");  
//        konekGSM(now);
      display.print("Lat:");
      display.println(fix.latitude(), 6);
//        konekGSM(now);
      display.print("Lon:");
      display.println(fix.longitude(), 6);
//        konekGSM(now);
      display.print("Speed: ");
      display.println(fix.speed_kph());
//        konekGSM(now);
      display.print("Alt: ");
      display.println(fix.altitude()); 
//        konekGSM(now);    
    display.display();
    lastScreenUpdate = now;
  }
#endif // USE_SCREEN  
//}
}
