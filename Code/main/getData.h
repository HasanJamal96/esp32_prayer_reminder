DynamicJsonDocument doc(60000);

const char* prayerNames[] = {"Fajr", "Dhuhr", "Asr", "Maghrib", "Isha"};

void getDataFromAPI() {
  if(WiFi.status() != WL_CONNECTED) {
    #if(DEBUG_API == true && DEBUG_MAIN == true)
      Serial.printf("[API] No internet conenection\n");
    #endif
    events.send("{\"s\":\"E\",\"r\":\"Wi-Fi not connected\"}","download",millis());
    return;
  }
  HTTPClient http;
  http.useHTTP10(true);
  for(uint8_t i=1; i<13; i++) { // 12 months a year
    String c = String(i);
    char p[150];
    String s="";
    strcpy(p, BASE_API_START);
    for(uint8_t i=0; i<strlen(structLocation.city); i++) {
      if(structLocation.city[i] != ' ')
        s += structLocation.city[i];
      else
        s += "%20";
    }
    strcat(p, s.c_str());
    strcat(p, BASE_API_MID);
    s = "";
    for(uint8_t i=0; i<strlen(structLocation.country); i++) {
      if(structLocation.city[i] != ' ')
        s += structLocation.country[i];
      else
        s += "%20";
    }
    strcat(p, s.c_str());
    strcat(p, BASE_API_END);
    strcat(p, c.c_str());
    http.begin(p);
    int httpResponseCode = http.GET();
    if(httpResponseCode == 200) {
      String payload = "";
       // 1 month data is approx 50k bytes
      deserializeJson(doc, http.getStream());
      JsonArray days = doc["data"];
      uint8_t daysCount = days.size();
      payload = "{";
      for(uint8_t d=0; d<daysCount; d++) {
        payload += "\"" + String(d+1) + "\":[";
        for(uint8_t x=0; x<5; x++) {
          payload += "\"";
          const String prayerC = days[d]["timings"][prayerNames[x]];
          payload += prayerC.substring(0,5);
          payload += "\",";
        }
        payload += "\"";
        const String sunRise = days[d]["timings"]["Sunrise"];
        payload += sunRise.substring(0,5);
        payload += "\",\"";
        const String sunSet = days[d]["timings"]["Sunset"];
        payload += sunSet.substring(0,5);
        payload += "\",\"";
        const String midNight = days[d]["timings"]["Midnight"];
        payload += midNight.substring(0,5);
        payload += "\"";
        if(d == daysCount-1)
          payload += "]";
        else
          payload += "],";
      }
      payload += "}";
      c = "/" + c + ".txt";
      if(savePrayerData(c.c_str(), payload)) {
        char s[20];
        strcpy(s, "{\"s\":\"C\",\"r\":\"");
        String per = String(map(i, 1, 12, 1, 100));
        strcat(s, per.c_str());
        strcat(s, "\"}");
        events.send(s, "download", millis());
      }
      else {
        events.send("{\"s\":\"E\",\"r\":\"Unable to save\"}","download",millis());
        break;
      }
    }
    http.end();
  }
}
