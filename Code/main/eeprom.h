void initEEPROM() {
  EEPROM.begin(EEPROM_SIZE);
}

void saveCity() {
  EEPROM.writeString(EEPROM_COUNTRY, structLocation.country);
  EEPROM.writeString(EEPROM_CITY,    structLocation.city);
  EEPROM.commit();
}

void readCity() {
  EEPROM.readString(EEPROM_COUNTRY, structLocation.country, 50);
  EEPROM.readString(EEPROM_CITY,    structLocation.city, 50);
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] Country: ");
    Serial.print(structLocation.country);
    Serial.printf(" City: ");
    Serial.println(structLocation.city);
  #endif
}


void saveWifiSettings() {
  EEPROM.writeString(EEPROM_WIFI_NAME, structInternet.wifiName);
  EEPROM.writeString(EEPROM_WIFI_PASS, structInternet.wifiPass);
  EEPROM.commit();
}

void readWifiSettings() {
  EEPROM.readString(EEPROM_WIFI_NAME, structInternet.wifiName, 30);
  EEPROM.readString(EEPROM_WIFI_PASS, structInternet.wifiPass, 30);
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] SSID: ");
    Serial.print(structInternet.wifiName);
    Serial.printf(" PASS: ");
    Serial.println(structInternet.wifiPass);
  #endif
}

void resetEEPROM() {
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] Resetting\n");
  #endif
  
  for(uint16_t i=0; i<EEPROM_SIZE; i++)
    EEPROM.write(i, 0);
  EEPROM.write(EEPROM_MAGIC_NUMBER, MAGIC_NUMBER);
  EEPROM.commit();
  
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] Successful reset\n");
  #endif
}


void readonfigurations() {
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] Reading configuration\n");
  #endif
  
  uint8_t magicNumber = EEPROM.read(EEPROM_MAGIC_NUMBER);
  if(magicNumber != MAGIC_NUMBER) {
    resetEEPROM();
  }
  else {
    readCity();
    readWifiSettings();
  }

  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[EEPROM] Reading complete\n");
  #endif
}
