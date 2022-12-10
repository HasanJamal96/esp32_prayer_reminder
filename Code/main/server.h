class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler(){
    serverRoutes();
  }
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request){
    if(request->url() == "/connecttest.txt" || request->url() == "/redirect" || request->url() == "/generate_204" || request->url() == "/fwlink" || request->url() == "/hotspot-detect.html")
      request->redirect("http://8.8.8.8/index.html");
    else
      request->send(SPIFFS, "/index.html", "text/html");
  }
};


void serverRoutes() {
  server.on("/index.html", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/jquery.js", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/jquery.js", "text/javascriptml");
  });

  server.on("/countries.json", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/countries.json", "application/json");
  });

  server.on("/bg.jpg", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send(SPIFFS, "/bg.jpg", "image/x-icon");
  });

  server.on("/set_city",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec;
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    uint8_t index1 = rec.indexOf(',', 0);
    strcpy(structLocation.country, rec.substring(0,index1).c_str());
    strcpy(structLocation.city, rec.substring(index1+1,len).c_str());
    saveCity();
    request->send(200, "text/plain", "ok");
    char s[50];
    strcpy(s, structLocation.country);
    strcat(s, ",");
    strcat(s, structLocation.city);
    events.send(s,"info",millis());
  });

  server.on("/set_time",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    if(rtcWorking) {
      String rec;
      for(int i=0; i<len; i++)
        rec += (char)data[i];    
      uint8_t index1 = rec.indexOf(',', 0);
      String timeData = rec.substring(0,index1);
      String dateData = rec.substring(index1+1,len);
      int year   = dateData.substring(0,4).toInt();
      int month  = dateData.substring(5,7).toInt();
      int day    = dateData.substring(8,10).toInt();
      int hour   = timeData.substring(0,2).toInt();
      int minute = timeData.substring(3,5).toInt();
      int second = timeData.substring(6,8).toInt();
      setRTCTime(year, month, day, hour, minute, second);
      request->send(200, "text/plain", "ok");
    }
    else
      request->send(200, "text/plain", "failed");
  });

  server.on("/connect_wifi",HTTP_POST,[](AsyncWebServerRequest * request){},NULL,[](AsyncWebServerRequest * request, uint8_t *data, size_t len, size_t index, size_t total){
    String rec;
    for(int i=0; i<len; i++)
      rec += (char)data[i];
    uint8_t index1 = rec.indexOf(',', 0);
    strcpy(structInternet.wifiName, rec.substring(0,index1).c_str());
    strcpy(structInternet.wifiPass, rec.substring(index1+1,len).c_str());
    structInternet.reconnect = true;
    structInternet.stopConnecting = false;
    saveWifiSettings();
    request->send(200, "text/plain", "ok");
  });

  server.on("/scan_wifi",HTTP_GET,[](AsyncWebServerRequest * request) {
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] Scanning\n");
    #endif
    request->send(200, "text/plain", "ok");
    WiFi.scanNetworks(true, false, false, 100);
  });
  server.on("/prayer_data",HTTP_GET,[](AsyncWebServerRequest * request) {
    startDownload = true;
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] StartDownload\n");
    #endif
    request->send(200, "text/plain", "ok");
  });
  
  events.onConnect([](AsyncEventSourceClient *client){
    char s[80];
    if(structInternet.state == CONNECTED) {
      strcpy(s, "{\"s\":\"0\",\"n\":\"");
      strcat(s, structInternet.wifiName);
      strcat(s, "\"}");
    }
    else if(structInternet.state == DISCONNECTED) {
      strcpy(s, "{\"s\":\"1\",\"r\":");
      if(structInternet.disconnectReasonId == 202)
        strcat(s, "\"Athentication failed\"}");
      else if(structInternet.disconnectReasonId == 201)
        strcat(s, "\"Not in range\"}");
      else if(structInternet.disconnectReasonId == 3)
        strcat(s, "\"Not in range\"}");
      else
        strcat(s, "\"Unknown\"}");
    }
    else if(structInternet.state == CONNECTING) {
      strcpy(s, "{\"s\":\"2\",\"n\":\"");
      strcat(s, structInternet.wifiName);
      strcat(s, "\"}");
    }
    events.send(s,"wifi_status",millis());
    strcpy(s, structLocation.country);
    strcat(s, ",");
    strcat(s, structLocation.city);
    events.send(s,"info",millis());
  });
}


void startServer() {
  startAp();
  server.addHandler(&events);
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);
  dnsServer.setErrorReplyCode(DNSReplyCode::NoError);
  dnsServer.start(53, "*", WiFi.softAPIP());
  serverRoutes();
  server.begin();
  structServer = RUNNING;
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
    Serial.printf("[Server] Started\n");
  #endif
}

void stopServer() {
  stopAp();
  dnsServer.stop();
  server.end();
  structServer = STOPPED;
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
    Serial.printf("[Server] Stopped\n");
  #endif
}
