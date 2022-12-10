#include <Arduino.h>
#include <stdio.h>
#include <FreeRTOS.h>
#include <driver/i2s.h>
#include "config.h"
#include "RTClib.h"
#include <EEPROM.h>
#include <DNSServer.h>
#include "WiFi.h"
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"
#include "I2SOutput.h"
#include "WAVFileReader.h"
#include "TimeLib.h"
#include <HTTPClient.h>
#include <ArduinoJson.h>

#ifdef USING_PCB
  #include <virtuabotixRTC.h>
  #define RTC_CLK   22
  #define RTC_DATA  21
  #define RTC_CE    15
  const uint8_t mth_key[12] = {1,3,3,0,2,5,0,3,6,1,4,6};
  virtuabotixRTC myRtc(RTC_CLK, RTC_DATA, RTC_CE);
#else
  RTC_DS3231      myRtc;
#endif


loc_t           structLocation;
internet_t      structInternet;
device_mode_t   deviceMode    = SETUP;
server_status_t structServer  = STOPPED;

voltage_status_t    currentVoltageStatus = LOW_VOLTAGE;
voltage_status_t    lastVoltageStatus = LOW_VOLTAGE;
unsigned long lastVoltageRead = 0;

IPAddress        apIP(8,8,8,8);
DNSServer        dnsServer;
AsyncWebServer   server(80);
AsyncEventSource events("/events");

DynamicJsonDocument doc(60000);

void serverRoutes();
void startServer();

unsigned long currentUnixTime;
int currentYear          = 1970;
int currentSelectedMonth = 0;
int currentSelectedDay = 0;
int lastSelectedMonth = 0;
int lastSelectedDay = 0;
int currentHour = 0;
int currentMinute = 0;
char currentPrayerTime[6];

unsigned long lastRTCUpdate = 0;
tmElements_t my_time;
#include "memory.h"
#include "getData.h"
#include "internet.h"
#include "eeprom.h"
#include "rtc.h"
#include "server.h"
#include "button.h"
#include "audio.h"

Button  buttonMode(MODE_BUTTON, 500);
Button  buttonPrayer(PRAYER_BUTTON, 500);
Button  buttonExtra(EXTRA_BUTTON, 500);

#define SD_MODE_PIN 17
i2s_pin_config_t i2s_speaker_pins = {
  .bck_io_num = GPIO_NUM_27,  // BCLK
  .ws_io_num = GPIO_NUM_14,   // LRCL
  .data_out_num = GPIO_NUM_26,// DIN
  .data_in_num = I2S_PIN_NO_CHANGE
};



unsigned long convertStringToUnix(String string, bool nextDay) {
  int h, m;
  unsigned long ut;
  h = string.substring(0,2).toInt();
  m = string.substring(3,5).toInt();
  my_time.Second = 0;
  my_time.Hour   = h;
  my_time.Minute = m;
  my_time.Day    = currentSelectedDay;
  my_time.Month  = currentSelectedMonth;
  my_time.Year   = currentYear - 1970;
  ut = makeTime(my_time);
  if(h == 0 && nextDay) {
    ut += (24*60*60);
  }
  return ut;
}


void play_task(void *param) {
  Output *output = new I2SOutput(I2S_NUM_0, i2s_speaker_pins);
  unsigned long prayersUnixTime[5];
  while (true){
    if(buttonPrayerPressed) {
      buttonPrayerPressed = false;
      if(prayerDataAvailable) {
        String p = String(currentSelectedDay);
        unsigned long sunRiseUnix  = convertStringToUnix(doc[p][5], false);
        unsigned long sunSetUnix   = convertStringToUnix(doc[p][6], false);
        unsigned long midNightUnix = convertStringToUnix(doc[p][7], true);
        /*Serial.print("Current Unix: ");
        Serial.println(currentUnixTime);
        Serial.print(" sunset Unix: ");
        Serial.print(sunSetUnix);
        Serial.print(" sunrise Unix: ");
        Serial.println(sunRiseUnix);*/
        for(uint8_t i=0; i<5; i++)
          prayersUnixTime[i] = convertStringToUnix(doc[p][i], false);

        bool prayerFound = false;
        for(uint8_t i=0; i<5; i++) {
          /*Serial.print("Current Unix: ");
          Serial.print(currentUnixTime);
          Serial.print(" Prayer Unix: ");
          Serial.println(prayersUnixTime[i]);*/
          
          switch(i) {
            case 0:
              if(currentUnixTime >= prayersUnixTime[i] && currentUnixTime < sunRiseUnix) {
                #if (DEBUG_MAIN == true)
                  Serial.println("[Prayer] Fajr");
                #endif
                playingAudio = true;
                play(output, "/spiffs/Fajr.wav");
                prayerFound = true;
              }
              break;
            case 1:
              if(currentUnixTime >= prayersUnixTime[i] && currentUnixTime < (prayersUnixTime[i+1] - (20*60*1000))) {
                #if (DEBUG_MAIN == true)
                  Serial.println("[Prayer] Duhr");
                #endif
                playingAudio = true;
                play(output, "/spiffs/Duhr.wav");
                prayerFound = true;
              }
              break;
            case 2:
              if(currentUnixTime >= prayersUnixTime[i] && currentUnixTime < sunSetUnix) {
                #if (DEBUG_MAIN == true)
                  Serial.println("[Prayer] Asr");
                #endif
                playingAudio = true;
                play(output, "/spiffs/Asr.wav");
                prayerFound = true;
              }
              break;
            case 3:
              if(currentUnixTime >= prayersUnixTime[i] && currentUnixTime < (prayersUnixTime[i+1] - (10*60*1000))) {
                #if (DEBUG_MAIN == true)
                  Serial.println("[Prayer] Maghrib");
                #endif
                play(output, "/spiffs/Maghrib.wav");
                playingAudio = true;
                prayerFound = true;
              }
              break;
            case 4:
              if(currentUnixTime >= prayersUnixTime[i] && currentUnixTime < midNightUnix) {
                #if (DEBUG_MAIN == true)
                  Serial.println("[Prayer] Isha");
                #endif
                playingAudio = true;
                play(output, "/spiffs/Isha.wav");
                prayerFound = true;
              }
              break;
          }
        }
        if(!prayerFound) {
          #if (DEBUG_MAIN == true)
            Serial.println("[Prayer] No Prayer time");
          #endif
          playingAudio = true;
          play(output, "/spiffs/NoPrayer.wav");
        }
      }
    }
    else if(buttonExtraPressed) {
      buttonExtraPressed = false;
      currentAudioFileIndex += 1;
      if(currentAudioFileIndex >= TOTAL_FILES) {
        currentAudioFileIndex = 0;
      }
      playingAudio = true;
      play(output, fileNamesList[currentAudioFileIndex]);
    }
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}


void setup() {
  #if (DEBUG_MAIN == true)
    Serial.begin(BAUDRATE);
    Serial.printf("[Main] Setup start\n");
    Serial.printf("[Main] Prayer reminder\n");
    Serial.printf("[Main] Version: ");
    Serial.println(VERSION);
  #endif

  initEEPROM();
  readonfigurations();
  beginRTC();

  buttonPrayer.begin();
  buttonMode.begin();
  buttonExtra.begin();

  pinMode(STATUS_LED, OUTPUT);
  pinMode(BATTERY_HIGH_LED, OUTPUT);
  pinMode(BATTERY_LOW_LED, OUTPUT);
  pinMode(BATTERY_MEASURE_PIN, INPUT);
  pinMode(SD_MODE_PIN, OUTPUT);
  digitalWrite(SD_MODE_PIN, HIGH); // HIGH Right channel, LOW AMP will shutdown(No Audio)
  
  delay(1000);

  if(buttonMode.read())
    deviceMode = SETUP;
  else
    deviceMode = WATCH;
  
  mountSPIFFS();

  if(deviceMode == SETUP) {
    #if (DEBUG_MAIN == true)
      Serial.printf("[Main] Device is in setup mode\n");
    #endif
    digitalWrite(STATUS_LED, HIGH);
    initInternet();
    delay(200);
    startServer();
  }
  
  if(deviceMode == WATCH) {
    #if (DEBUG_MAIN == true)
      Serial.printf("[Main] Device is in watch mode\n");
      Serial.printf("[Main] Device must have prayer data stored and RTC configured\n");
    #endif
    digitalWrite(STATUS_LED, LOW);
    updateRTC();
    currentSelectedMonth = lastSelectedMonth;
    currentSelectedDay   = lastSelectedDay;
    updatePrayerData();
    xTaskCreate(play_task, "PLAYTASK", 4096, NULL, 0, NULL);
  }
  
  #if (DEBUG_MAIN == true)
    Serial.printf("[Main] Setup complete\n");
  #endif
}

void restartESP() {
  #if (DEBUG_MAIN == true)
    Serial.printf("[Main] Restarting in \n");
  #endif
  for(int8_t i=3; i>=0; i--) {
    #if (DEBUG_MAIN == true)
      Serial.printf("%d\n", i);
    #endif
    vTaskDelay(1000);
  }
  ESP.restart();
}

float getCurrentVoltage() {
  float rawValue = analogRead(BATTERY_MEASURE_PIN);
  rawValue = (rawValue * 3.3) / 4095;
  return rawValue / (RESISTOR_2/(RESISTOR_1+RESISTOR_2)); 
}


void loop() {
  if(buttonMode.read() && deviceMode == WATCH)
    restartESP();
  else if(!buttonMode.read() && deviceMode == SETUP)
    restartESP();
    
  if(deviceMode == SETUP) {
    internetHandle();
    if(structServer == RUNNING)
      dnsServer.processNextRequest();

    if(startDownload) {
      getDataFromAPI();
      startDownload = false;
    }
  }
  else {
    if(millis() - lastRTCUpdate >= 5000 && (!playingAudio)) {
      updateRTC();
      if(lastSelectedMonth != currentSelectedMonth) {
        currentSelectedMonth = lastSelectedMonth;
        updatePrayerData();
      }
    }
    buttonPrayer.read();
    buttonExtra.read();
    if(buttonPrayer.pressedFor(300)) {
      if(playingAudio) {
        playingAudio = false;
        vTaskDelay(500);
      }
      buttonPrayerPressed = true;
    }
    else if(buttonExtra.pressedFor(300)) {
      if(playingAudio) {
        playingAudio = false;
        vTaskDelay(500);
      }
      buttonExtraPressed = true;
    }
  }
  if(millis() - lastVoltageRead >= 10000 && (!playingAudio)) {
    if(getCurrentVoltage() >= FULL_THRESHOLD_VOLTAGE) {
      currentVoltageStatus = FULL_VOLTAGE;
    }
    else if(getCurrentVoltage() < FULL_THRESHOLD_VOLTAGE && getCurrentVoltage() > LOW_THRESHOLD_VOLTAGE) {
      currentVoltageStatus = MID_VOLTAGE;
    }
    else if(getCurrentVoltage() <= LOW_THRESHOLD_VOLTAGE) {
      currentVoltageStatus = LOW_VOLTAGE;
    }
    if(currentVoltageStatus != lastVoltageStatus) {
      if(currentVoltageStatus == LOW_VOLTAGE) {
        digitalWrite(BATTERY_LOW_LED, HIGH);
        digitalWrite(BATTERY_HIGH_LED, LOW);
      }
      else if(currentVoltageStatus == MID_VOLTAGE) {
        digitalWrite(BATTERY_LOW_LED, LOW);
        digitalWrite(BATTERY_LOW_LED, LOW);
        
      }
      else if(currentVoltageStatus == FULL_VOLTAGE) {
        digitalWrite(BATTERY_LOW_LED, LOW);
        digitalWrite(BATTERY_HIGH_LED, HIGH);
      }
      lastVoltageStatus = currentVoltageStatus;
    }
    lastVoltageRead = millis();
  }
}
