
#define __WIFI_ROUTES_C__

#include "wifi_routes.h"

#include <ArduinoJson.h>

#include "../storage/wifi_credential_storage.h"
#include "AsyncJson.h"
#include "html.h"
#include "webserver_config.h"

void GET_wifi_page(AsyncWebServerRequest *request) {
  String wifiHtml = WIFI_HTML;

  // TODO: replace something.. otherwise, pass in the WIFI_HTML directly!!

  request->send(200, HTTP_CONTENT_TYPE_HTML, wifiHtml.c_str());
}

void GET_wifi_scan_results_handler(AsyncWebServerRequest *request) {
  // TODO: include
  extern uint32_t iizi_wifi_scan_started;
  const auto is_scanning = iizi_wifi_scan_started ? true : false;
  auto n_networks        = WiFi.scanComplete();
  if (n_networks < 0 || is_scanning) {
    n_networks = 0;
  }

  const auto json_size =
      // root object
      JSON_OBJECT_SIZE(2 /* networks, n_networks */) +
      // the networks array
      JSON_ARRAY_SIZE(n_networks) +
      // the nested arrays..
      n_networks * JSON_ARRAY_SIZE(2 /* ssid, rssi */) +
      // some space for the ssid names?
      n_networks * 20;

  Serial.printf("json_size: %d\n", json_size);

  DynamicJsonDocument json(json_size);
  json["n_networks"] = n_networks;

  const auto networks = json.createNestedArray("networks");
  for (uint8_t i = 0; i < n_networks; ++i) {
    String ssid = WiFi.SSID(i);

#if WIFI_SCAN_REMOVE_DUPLICATES
    boolean duplicated = false;
    for (auto v : networks) {
      if (v.getElement(0).as<String>() == ssid) {
        duplicated = true;
        break;
      }
    }

    if (duplicated) {
      // Serial.printf("ignore %s ..\n", ssid.c_str());
      continue;
    }
#endif

    const auto tmp_arr = networks.createNestedArray();
    tmp_arr.add(ssid);
    tmp_arr.add(WiFi.RSSI(i));
  }

  const auto response = request->beginResponseStream(HTTP_CONTENT_TYPE_JSON);
  serializeJson(json, *response);
  request->send(response);
}

void GET_wifi_query_handler(AsyncWebServerRequest *request) {
  // const auto json_size = wcs_calculate_json_array_size(false);
  // somehow with "false" its size is too small and we loose infos
  // TODO: check why the info is getting copied instead of being used "in
  // place"??
  const auto json_size = wcs_calculate_json_array_size(true);
  DynamicJsonDocument json(json_size);

  const auto credentials = json.createNestedArray("credentials");

  wcs_to_json_array(credentials);

  json["n_credentials"] = credentials.size();

  const auto response = request->beginResponseStream(HTTP_CONTENT_TYPE_JSON);
  serializeJson(json, *response);
  request->send(response);
}

static auto *POST_wifi_save_handler = new AsyncCallbackJsonWebHandler(
    EAF_WM_WEBSERVER_PREFIX "/wifi-save",
    [](AsyncWebServerRequest *request, JsonVariant &json) {
      auto creds = json["creds"].as<JsonArray>();
      if (creds.isNull()) {
        return request->send(200);
      }

      Serial.printf("/wifi-save: creds size: %d\n", creds.size());

      wcs_read_from_array(creds);
      wcs_save();

      request->send(200);
    });

void wifi_routes_add(AsyncWebServer &webserver) {
  webserver.on(EAF_WM_WEBSERVER_PREFIX "/wifi", HTTP_GET, GET_wifi_page);

  webserver.on(EAF_WM_WEBSERVER_PREFIX "/wifi-scan-results", HTTP_GET,
               GET_wifi_scan_results_handler);

  webserver.on(EAF_WM_WEBSERVER_PREFIX "/wifi-start-scan", HTTP_POST,
               [](AsyncWebServerRequest *request) {
                 // TODO: include..
                 extern bool iizi_wifi_should_start_scan;
                 iizi_wifi_should_start_scan = true;
                 request->send(200);
               });

  webserver.on(EAF_WM_WEBSERVER_PREFIX "/wifi-query", HTTP_GET,
               GET_wifi_query_handler);

  POST_wifi_save_handler->setMethod(HTTP_POST);
  webserver.addHandler(POST_wifi_save_handler);
}
