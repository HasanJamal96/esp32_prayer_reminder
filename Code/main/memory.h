void mountSPIFFS() {
  if(!SPIFFS.begin(true)) {
    #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
      Serial.printf("[SPIFFS] Mount failed\n");
    #endif
    return;
  }
  #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
    Serial.printf("[SPIFFS] Mount success\n");
  #endif
}

bool savePrayerData(const char *filename, String fileData) {
  File file = SPIFFS.open(filename, "w");
  bool saved = false;
  if(file){
    if(file.print(fileData)) {
      saved = true;
      #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
        Serial.println("[SPIFFS] File saved");
      #endif
    }
    #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
      else {
        Serial.println("[SPIFFS] Failed to save file");
      }
    #endif
  }
  else {
    file.print("");
  }
  file.close();
  return saved;
}


String readPrayerData(const char *filename) {
  File file = SPIFFS.open(filename, FILE_READ);
  String ReadData = "";
  if(file){
    if(file.size()>10){
      while (file.available())
        ReadData += file.readString();
    }
  }
  file.close();
  return ReadData;
}

void updatePrayerData() {
  String fileName = "/" + String(currentSelectedMonth) + ".txt";
  String fileData = readPrayerData(fileName.c_str());
  if(fileData == "") {
    prayerDataAvailable = false;
  }
  else {
    prayerDataAvailable = true;
    deserializeJson(currentMonthData, fileData);
    #if(DEBUG_EEPROM == true && DEBUG_MAIN == true)
      Serial.printf("[SPIFFS] Json data: ");
      serializeJson(currentMonthData, Serial);
      Serial.printf("\n");
    #endif
  }
}
