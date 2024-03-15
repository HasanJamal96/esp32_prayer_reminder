# esp32_prayer_reminder
This project is a prayer reminder system built using an ESP32. It assists users in staying informed about prayer times and offers a convenient way to play surahs.

## Hardware
  - ESP32
  - MAX98357
  - 2 push button
  - 1 toggle switch
  - RTC DS3231
  - Speaker

## Working:
### Mode:
  - **Configuration Mode:**
      1. Users connect to a dedicated ESP32 access point and configure settings through a captive portal.
      2. Set RTC time and date for accurate prayer calculations.
      3. Select the user's country for relevant prayer time data.
      4. Connect ESP32 rto Wi-Fi and download prayer timings from the internet.
  - **Normal Mode:**
      1. Provides immediate access to prayer information and surah playback.
      2. Announce the current prayer time upon pressing the first push button.
      3. Play a stored surah from memory by pressing the second push button.
