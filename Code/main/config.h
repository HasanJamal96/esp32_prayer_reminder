//#define DEBUG_MAIN      true
//#define DEBUG_INTERNET  true
//#define DEBUG_EEPROM    true
//#define DEBUG_SPIFFS    true
//#define DEBUG_RTC       true
//#define DEBUG_API       true

#define USING_PCB


#if (DEBUG_MAIN == true)
#define BAUDRATE  115200
#endif

const char VERSION[] = "0.1";

const char* BASE_API_START = "http://api.aladhan.com/v1/calendarByCity?city=";
const char* BASE_API_MID = "&country=";
const char* BASE_API_END = "&month=";

#define MODE_BUTTON 4
#define PRAYER_BUTTON 5

bool rtcWorking = false;
bool startDwonload = false;
bool prayerDataAvailable = false;

typedef enum: uint8_t {
  SETUP = 0,
  WATCH = 1,
}device_mode_t;



typedef struct {
  char country[50];
  char city[50];
}loc_t;


typedef enum: uint8_t {
  RUNNING = 0,
  STOPPED = 1,
}server_status_t;


typedef enum: uint8_t {
  CONNECTING   = 0,
  CONNECTED    = 1,
  DISCONNECTED = 2,
}internet_status_t;

typedef struct {
  char wifiName[30] = "";
  char wifiPass[30] = "";
  char apName[30]   = "Prayer Hotspot ";
  char apPass[10]   = "123456789";

  uint8_t disconnectReasonId = 0;
  String  scanResult = "";
  internet_status_t state;
  bool stopConnecting  = false;
  bool reconnect       = true;
  char macAddress[20];
  unsigned long  lastConnectionTry;
  const uint16_t RETRY_AFTER = 20000;
}internet_t;


const int     EEPROM_SIZE  = 1024;
const uint8_t MAGIC_NUMBER = 100;


// EEPROM Locations
const int EEPROM_MAGIC_NUMBER   = 0;
const int EEPROM_WIFI_NAME      = 1;
const int EEPROM_WIFI_PASS      = EEPROM_WIFI_NAME + 30;
const int EEPROM_COUNTRY        = EEPROM_WIFI_PASS + 30;
const int EEPROM_CITY           = EEPROM_COUNTRY   + 50;
const int EEPROM_DEVICE_STATE   = EEPROM_CITY      + 50;



// Audio File names
const char *FAJR = "Fajr.wav";
const char *DHUR = "Dhur.wav";
const char *ASR = "Asr.wav";
const char *MAGHRIB = "Maghrib.wav";
const char *ISHA = "Isha.wav";
const char *NO_PRAYER = "NoPrayer.wav";
