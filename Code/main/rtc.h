void beginRTC() {
#ifndef USING_PCB
  if (! myRtc.begin()) {
    rtcWorking = false;
    #if(DEBUG_MAIN == true && DEBUG_RTC == true)
      Serial.printf("[RTC] Not found\n");
    #endif
  }
  else {
    rtcWorking = true;
    //myRtc.adjust(DateTime(__DATE__, __TIME__));
    #if(DEBUG_MAIN == true && DEBUG_RTC == true)
      Serial.printf("[RTC] Found\n");
    #endif
  }
#else
  rtcWorking = true;
  #if(DEBUG_MAIN == true && DEBUG_RTC == true)
    Serial.printf("[RTC] Assuming RTC is connected and working\n");
  #endif
#endif
}



void setRTCTime(int year, int month, int day, int hour, int minute, int second) {
#ifdef USING_PCB
  int y = year % 100;
  int dow = ((y /4) + day + mth_key[month-1] + 4 + y) % 4;
  myRtc.setDS1302Time(second, minute, hour, dow, day, month, year);
#else
  myRtc.adjust(DateTime(year, month, day, hour, minute, second));
#endif
#if(DEBUG_MAIN == true && DEBUG_RTC == true)
  Serial.printf("[RTC] Updated\n");
#endif
}


void updateRTC() {
#ifdef USING_PCB
  myRtc.updateTime();
  currentYear       = myRtc.year;
  lastSelectedMonth = myRtc.month;
  lastSelectedDay   = myRtc.dayofmonth;
  currentHour       = myRtc.hours;
  currentMinute     = myRtc.minutes;
  my_time.Second = myRtc.seconds;
  my_time.Hour   = currentHour;
  my_time.Minute = currentMinute;
  my_time.Day    = lastSelectedDay;
  my_time.Month  = lastSelectedMonth;
  my_time.Year   = currentYear - 1970;
  currentUnixTime = makeTime(my_time);
#else
  DateTime now = myRtc.now();
  currentUnixTime   = now.unixtime();
  currentYear       = now.year();
  lastSelectedMonth = now.month();
  lastSelectedDay   = now.day();
  currentHour       = now.hour();
  currentMinute     = now.minute();
#endif
  if(lastSelectedMonth > 12)
    rtcWorking = false;
  #if(DEBUG_MAIN == true && DEBUG_RTC == true)
    Serial.printf("[RTC]%d/%d/%d %d:%d\n", currentYear, lastSelectedMonth, lastSelectedDay, currentHour, currentMinute);
  #endif
  lastRTCUpdate = millis();
}
