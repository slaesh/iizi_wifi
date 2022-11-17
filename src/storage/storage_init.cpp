#define __STORAGE_INIT_C__

#include <LittleFS.h>

#include "FS.h"

#define FileFS LittleFS

bool storage_init() {
  const auto fsInitOk = FileFS.begin(true);
  if (fsInitOk) return true;

  Serial.println(F("SPIFFS/LittleFS failed! Already tried formatting."));

  if (!FileFS.begin()) {
    // prevents debug info from the library to hide err message.
    vTaskDelay(100 / portTICK_PERIOD_MS);

    Serial.println(
        F("LittleFS failed!. Please use SPIFFS or EEPROM. Stay forever"));

    while (true) {
      vTaskDelay(100 / portTICK_PERIOD_MS);
    }
  }

  return true;
}
