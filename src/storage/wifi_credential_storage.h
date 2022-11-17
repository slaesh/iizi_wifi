#ifndef __WIFI_CREDENTIAL_STORAGE_H__
#define __WIFI_CREDENTIAL_STORAGE_H__

#include <ArduinoJson.h>

#define MAX_WIFI_CREDENTIALS (4)
#define MAX_WIFI_SSID_LEN    (32)
#define MAX_WIFI_PASS_LEN    (32)

typedef struct {
  const char *ssid;
  const char *pass;
} t_wifi_credentials;

extern t_wifi_credentials wifi_credentials[MAX_WIFI_CREDENTIALS];

void wcs_init();
uint32_t wcs_calculate_json_array_size(boolean including_memory_for_values);
boolean wcs_to_json_array(const JsonArray &json_array);
boolean wcs_read_from_array(const JsonArray &wifi_creds);
boolean wcs_save();
boolean wcs_read();
uint8_t wcs_stored_credentials_count();

#endif  // __WIFI_CREDENTIAL_STORAGE_H__
