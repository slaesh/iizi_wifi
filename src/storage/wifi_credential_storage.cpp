#define __WIFI_CREDENTIAL_STORAGE_C__

#include "wifi_credential_storage.h"

#include <LittleFS.h>
#include <WiFi.h>

#include "FS.h"
#include "esp_wifi.h"
// #include "StreamUtils.h"

#define FileFS   LittleFS
#define WCS_FILE F("/__iizi.wifi")

static char ssids[MAX_WIFI_CREDENTIALS][MAX_WIFI_SSID_LEN + 1];
static char passswords[MAX_WIFI_CREDENTIALS][MAX_WIFI_PASS_LEN + 1];

t_wifi_credentials wifi_credentials[MAX_WIFI_CREDENTIALS] = {
    {
        .ssid = ssids[0],
        .pass = passswords[0],
    },
    {
        .ssid = ssids[1],
        .pass = passswords[1],
    },
    {
        .ssid = ssids[2],
        .pass = passswords[2],
    },
    {
        .ssid = ssids[3],
        .pass = passswords[3],
    },
};

uint32_t wcs_calculate_json_array_size(
    // only needed for deserialization
    boolean including_memory_for_values) {
  const auto n_wifi_creds = MAX_WIFI_CREDENTIALS;

  const auto base_size =
      // parent array
      JSON_ARRAY_SIZE(n_wifi_creds) +
      // nested arrays (ssid, pass)
      JSON_ARRAY_SIZE(2) * n_wifi_creds;

  if (!including_memory_for_values) {
    return base_size;
  }

  // TODO: calculate the actual value..
  const auto mem_for_key_and_values = n_wifi_creds * 80;

  return base_size + mem_for_key_and_values;
}

boolean wcs_to_json_array(const JsonArray &json_array) {
  for (uint8_t c = 0; c < MAX_WIFI_CREDENTIALS; ++c) {
    const auto creds = wifi_credentials[c];

    if (creds.ssid[0] == 0) continue;

    const auto a = json_array.createNestedArray();

    // order of entries needs to be [ssid, pass]
    a.add(creds.ssid);
    a.add(creds.pass);
  }

  return true;
}

boolean wcs_save() {
  File f = FileFS.open(WCS_FILE, "w", true);
  if (!f) {
    Serial.println("wcs_save(): cant open file??");
    return false;
  }

  const auto json_size = wcs_calculate_json_array_size(false);

  Serial.printf("wcs_save(): json_size: %d\n", json_size);

  DynamicJsonDocument json(json_size);
  const auto json_array = json.to<JsonArray>();

  wcs_to_json_array(json_array);

  const auto n_bytes_written = serializeJson(json, f);

  Serial.printf("wcs_save(): %d bytes written\n", n_bytes_written);

  f.flush();
  f.close();

  return true;
}

void str_n_copy_with_termination(char *dst, const char *src, size_t len) {
  strncpy(dst, src, len);

  // take care of termination!
  if (dst[len - 1] != 0) {
    dst[len - 1] = 0;
  }
}

void wcs_clear_all_creds() {
  for (uint8_t c = 0; c < MAX_WIFI_CREDENTIALS; ++c) {
    const auto creds = wifi_credentials[c];

    memset((void *)creds.ssid, 0, MAX_WIFI_SSID_LEN + 1);
    memset((void *)creds.pass, 0, MAX_WIFI_PASS_LEN + 1);
  }
}

boolean wcs_read_from_array(const JsonArray &wifi_creds) {
  wcs_clear_all_creds();

  if (wifi_creds.size() == 0) return true;

  uint8_t creds_idx = 0;

  for (auto creds : wifi_creds) {
    if (creds_idx >= MAX_WIFI_CREDENTIALS) break;
    if (creds.isNull() || creds.size() != 2) continue;

    const auto ssid = creds.getElement(0).as<String>();
    const auto pass = creds.getElement(1).as<String>();

    Serial.printf("wcs_read_from_array(): s(%s) :: p(%s)\n", ssid.c_str(),
                  pass.c_str());

    const auto c = wifi_credentials + creds_idx;

    char *cur_ssid = (char *)(c->ssid);
    str_n_copy_with_termination(cur_ssid, ssid.c_str(), MAX_WIFI_SSID_LEN + 1);

    char *cur_pass = (char *)(c->pass);
    str_n_copy_with_termination(cur_pass, pass.c_str(), MAX_WIFI_PASS_LEN + 1);

    ++creds_idx;
  }

  return true;
}

boolean wcs_read() {
  File f = FileFS.open(WCS_FILE, "r");
  if (!f) {
    Serial.println("wcs_read(): cant open file??");
    return false;
  }

  const auto json_doc_size = wcs_calculate_json_array_size(true);

  Serial.printf("wcs_read(): json_doc_size: %d\n", json_doc_size);

  DynamicJsonDocument json(json_doc_size);

  // ReadBufferingStream bufferingStream(f, 64);
  // const auto deserialize_error = deserializeJson(creds, bufferingStream);

  const auto deserialize_error = deserializeJson(json, f);
  if (deserialize_error) {
    Serial.print(F("wcs_read(): deserializeJson() failed with code "));
    Serial.println(deserialize_error.f_str());

    return false;
  }

  const auto wifi_creds = json.as<JsonArray>();
  return wcs_read_from_array(wifi_creds);
}

// WiFi.begin() has to be called before that this may work..
String getStoredWiFiSSID() {
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    return String();
  }

  wifi_ap_record_t info;

  if (!esp_wifi_sta_get_ap_info(&info)) {
    return String(reinterpret_cast<char *>(info.ssid));
  }

  wifi_config_t conf;

  esp_wifi_get_config(WIFI_IF_STA, &conf);

  return String(reinterpret_cast<char *>(conf.sta.ssid));
}

// WiFi.begin() has to be called before that this may work..
String getStoredWiFiPass() {
  if (WiFi.getMode() == WIFI_MODE_NULL) {
    return String();
  }

  wifi_config_t conf;

  esp_wifi_get_config(WIFI_IF_STA, &conf);

  return String(reinterpret_cast<char *>(conf.sta.password));
}

bool wcs_creds_already_stored(String ssid, String pass) {
  for (uint8_t c = 0; c < MAX_WIFI_CREDENTIALS; ++c) {
    const auto creds = wifi_credentials + c;

    const String _ssid = creds->ssid;
    if (_ssid != ssid) continue;

    const String _pass = creds->pass;
    if (_pass == pass) return true;
  }

  return false;
}

void wcs_add_credentials(String ssid, String pass) {
  for (uint8_t c = 0; c < MAX_WIFI_CREDENTIALS; ++c) {
    const auto creds = wifi_credentials + c;

    // already something stored?
    if (creds->ssid[0] != 0) continue;

    char *cur_ssid = (char *)(creds->ssid);
    str_n_copy_with_termination(cur_ssid, ssid.c_str(), MAX_WIFI_SSID_LEN + 1);

    char *cur_pass = (char *)(creds->pass);
    str_n_copy_with_termination(cur_pass, pass.c_str(), MAX_WIFI_PASS_LEN + 1);

    // we did it, done here!
    return;
  }
}

void wcs_init() {
  wcs_read();

  const auto ssid = getStoredWiFiSSID();
  const auto pass = getStoredWiFiPass();

  Serial.printf("ESP stored.. %s :: %s\n", ssid.c_str(), pass.c_str());

  // TODO: do we want to check the stored against our "internal" ones?

  const auto n_creds_stored = wcs_stored_credentials_count();
  Serial.printf("wcs_init(): found %d stored credentials\n", n_creds_stored);

  if (n_creds_stored >= MAX_WIFI_CREDENTIALS) return;
  if (wcs_creds_already_stored(ssid, pass)) return;

  Serial.println("wcs_init(): lets add the last known credentials!!");

  wcs_add_credentials(ssid, pass);
  wcs_save();
}

uint8_t wcs_stored_credentials_count() {
  uint8_t stored_cnt = 0;

  for (uint8_t c = 0; c < MAX_WIFI_CREDENTIALS; ++c) {
    const auto creds = wifi_credentials[c];

    if (creds.ssid[0] == 0) continue;

    ++stored_cnt;
  }

  return stored_cnt;
}
