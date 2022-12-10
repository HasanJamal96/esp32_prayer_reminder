//#define DEBUG_MAIN      true
//#define DEBUG_INTERNET  true
//#define DEBUG_EEPROM    true
//#define DEBUG_SPIFFS    true
//#define DEBUG_RTC       true
//#define DEBUG_API       true

#define USING_PCB //uncomment this line if you are using RTC DS1302


#if (DEBUG_MAIN == true)
#define BAUDRATE  115200
#endif

const char VERSION[] = "0.2";

const char* BASE_API_START = "http://api.aladhan.com/v1/calendarByCity?city=";
const char* BASE_API_MID = "&country=";
const char* BASE_API_END = "&month=";

#define MODE_BUTTON   39
#define PRAYER_BUTTON 36
#define EXTRA_BUTTON  35

#define BATTERY_MEASURE_PIN 34
const float FULL_THRESHOLD_VOLTAGE = 1.80;
const float LOW_THRESHOLD_VOLTAGE = 1.6;
const float RESISTOR_1 = 100000;
const float RESISTOR_2 = 100000;

#define STATUS_LED          2
#define BATTERY_HIGH_LED    4
#define BATTERY_LOW_LED     16

typedef enum: uint8_t {
  LOW_VOLTAGE =  0,
  FULL_VOLTAGE = 1,
  MID_VOLTAGE  = 2,
}voltage_status_t;

bool rtcWorking = false;
bool startDownload = false;
bool prayerDataAvailable = false;
bool playingAudio = false;

bool buttonPrayerPressed = false;
bool buttonExtraPressed = false;

const uint8_t TOTAL_FILES     = 3;
int8_t currentAudioFileIndex = -1;
const char *fileNamesList[TOTAL_FILES] = {"/spiffs/Salam.wav", "/spiffs/Al-Fatihah.wav", "/spiffs/Ayat Al-Kursi.wav"};
//const char *fileNamesList[TOTAL_FILES] = {"/spiffs/Salam.wav", "/spiffs/Maghrib.wav", "/spiffs/Fajr.wav"};

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
