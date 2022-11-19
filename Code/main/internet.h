void internetCallback(WiFiEvent_t event) {
  if(event == SYSTEM_EVENT_STA_CONNECTED || event == SYSTEM_EVENT_ETH_CONNECTED) {
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] Connected\n"); 
    #endif
    WiFi.setAutoReconnect(true);
    char wss[50];
    strcpy(wss, "{\"s\":\"0\",\"n\":\"");
    strcat(wss, structInternet.wifiName);
    strcat(wss, "\"}");
    events.send(wss, "wifi_status",millis());
    structInternet.disconnectReasonId = 0;
  }
  else if(event == SYSTEM_EVENT_STA_GOT_IP || event == SYSTEM_EVENT_ETH_GOT_IP) {
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] Got IP address: ");
       Serial.println(WiFi.localIP());
    #endif
    structInternet.state = CONNECTED;
  }
  else if(event == SYSTEM_EVENT_AP_STACONNECTED) {
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] AP client connected\n");
    #endif
    structInternet.stopConnecting = true;
  }
  else if(event == SYSTEM_EVENT_AP_STADISCONNECTED) {
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] AP client disconnected\n");
    #endif
    structInternet.stopConnecting = false;
  }
  else if(event == SYSTEM_EVENT_SCAN_DONE) {
    structInternet.scanResult = "[";
      int n = WiFi.scanComplete();
      if(n && n != -2){
        for (int i = 0; i < n; ++i)
          structInternet.scanResult += "\""+WiFi.SSID(i)+"\",";
        WiFi.scanDelete();
      }
      if(structInternet.scanResult.length() > 5)
        structInternet.scanResult = structInternet.scanResult.substring(0,structInternet.scanResult.length()-1);
      structInternet.scanResult += "]";
      #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
        Serial.printf("[Internet] Scan complete ");
        Serial.println(structInternet.scanResult);
      #endif
      events.send(structInternet.scanResult.c_str(),"scan_result",millis());
  }
}


void internetDisconnectCallback(WiFiEvent_t event, WiFiEventInfo_t info) {
  uint8_t res = info.disconnected.reason;
  char wss[50];
  strcpy(wss, "{\"s\":\"1\",\"n\":\"");
  structInternet.state = DISCONNECTED;
  if(res == 15 || res == 202) {
    structInternet.disconnectReasonId = 202; // "Authentication failed - Invalid password"
    strcat(wss, "Authentication failed - Invalid password");
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("Authentication failed - Invalid password\n");
    #endif
  }
  else if(res == 201) {
    structInternet.disconnectReasonId = 201; // "WiFi is not in range"
    strcat(wss, "WiFi is not in range");
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] WiFi is not in range\n");
    #endif
  }
  else if(res == 3) {
    structInternet.disconnectReasonId = 3; // "WiFi is not answering"
    strcat(wss, "WiFi is not answering");
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] WiFi is not answering\n");
    #endif
  }
  else {
    structInternet.disconnectReasonId = 255; // "unknown reason"
    strcat(wss, "unknown reason");
    #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[Internet] Unknown reason\n");
    #endif
  }
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
    Serial.printf("[Internet] Disconnected\n");
  #endif
  if(millis() - structInternet.lastConnectionTry >= 8000)
    WiFi.setAutoReconnect(false);
  strcat(wss, "\"}");
  events.send(wss, "wifi_status",millis());
}


void initInternet() {
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
    Serial.printf("[Internet] Initializing\n"); 
  #endif
  
  WiFi.disconnect(true, true);
  delay(50);
  WiFi.mode(WIFI_MODE_APSTA);
  WiFi.onEvent(internetCallback);
  WiFi.onEvent(internetDisconnectCallback, SYSTEM_EVENT_STA_DISCONNECTED);

  String mac = WiFi.macAddress();
  mac.replace(":","");
  strcpy(structInternet.macAddress, mac.c_str());
  strcat(structInternet.apName, (mac.substring(0,4).c_str()));

  structInternet.state = DISCONNECTED;
  
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
    Serial.printf("[Internet] Initialized\n");
  #endif
}


void connectInternet() {
  if(millis() - structInternet.lastConnectionTry >= structInternet.RETRY_AFTER || structInternet.reconnect) {
    if(structInternet.reconnect) {
      structInternet.reconnect = false;
    }
    if(strlen(structInternet.wifiName) !=  0) {
      WiFi.begin(structInternet.wifiName, structInternet.wifiPass);      
      structInternet.state == CONNECTING;
      #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
        Serial.printf("[Internet] Connecting to: ");
        Serial.println(structInternet.wifiName);
      #endif
      vTaskDelay(100);
      char wss[50];
      strcpy(wss, "{\"s\":\"2\",\"n\":\"");
      strcat(wss, structInternet.wifiName);
      strcat(wss, "\"}");
    }
    structInternet.lastConnectionTry = millis();
  }
}


void startAp() {
  WiFi.softAPdisconnect();
  vTaskDelay(200);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  vTaskDelay(200);
  WiFi.softAP(structInternet.apName, structInternet.apPass);
  vTaskDelay(200);
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[AP] Started\n");
  #endif
}


void stopAp() {
  WiFi.softAPdisconnect(true);
  #if(DEBUG_MAIN == true && DEBUG_INTERNET == true)
      Serial.printf("[AP] Stopped\n");
  #endif
}


void internetHandle() {
  if(structInternet.stopConnecting) {
    WiFi.setAutoReconnect(false);
    return;
  }
  if(structInternet.state == DISCONNECTED || structInternet.reconnect) {
    if(structInternet.disconnectReasonId != 202 && structInternet.state != CONNECTING)
      connectInternet();
  }
}
