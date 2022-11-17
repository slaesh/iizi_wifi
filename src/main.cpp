
#define _MAIN_C_

#include <Arduino.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include <esp_task_wdt.h>

#include "AsyncJson.h"
#include "ESPAsyncDNSServer.h"
#include "ESPAsyncWebServer.h"
#include "html.h"
#include "is_timer_expired.h"
#include "storage/custom_param_storage.h"
#include "storage/storage_init.h"
#include "storage/wifi_credential_storage.h"
#include "userspace/parameters.h"
#include "webserver/param_routes.h"
#include "webserver/wifi_routes.h"

AsyncWebServer asyncWebserver(80);

boolean isIp(const String &str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);

    if ((c != '.') && (c != ':') && ((c < '0') || (c > '9'))) {
      return false;
    }
  }

  return true;
}

String toStringIp(const IPAddress &ip) {
  String res = "";

  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }

  res += String(((ip >> 8 * 3)) & 0xFF);

  return res;
}

boolean captivePortal(AsyncWebServerRequest *request) {
  Serial.printf("request host IP = %s\n", request->host().c_str());

  String ourHostname = "slaesh-iwr-308398d1a644";

  // our hostname? thats fine!
  if (request->host() == ourHostname) return false;
  // our ip? thats fine too!
  if (isIp(request->host()) &&
      toStringIp(request->client()->localIP()) == request->host())
    return false;

  // .. otherwise redirect!
  String redirectRoute =
      String("http://") + toStringIp(request->client()->localIP());
  Serial.printf("redirecting to our root route! '%s'\n", redirectRoute.c_str());

  request->redirect(redirectRoute);

  return true;
}

unsigned long start_wifi_scan = 0;

void start_a_scan() {
  start_wifi_scan = millis();
  if (start_wifi_scan == 0) ++start_wifi_scan;

  Serial.println("scan start");

  WiFi.scanNetworks(
      // async
      true,
      // show hidden
      true,
      // passive=false
      false,
      // timeout per channel
      1000);

  Serial.println(".. should run in the background");
}

AsyncDNSServer dnsServer;

void init_dnsServer() {
  dnsServer.stop();

  dnsServer.setErrorReplyCode(AsyncDNSReplyCode::NoError);

  // redirect all dns request to our own ip!
  if (!dnsServer.start(53, "*", WiFi.softAPIP())) {
    Serial.println("CANT START DNS SERVER!!");
  }
}

void _wifiEvents(arduino_event_t *event) {
  switch (event->event_id) {
    case ARDUINO_EVENT_WIFI_SCAN_DONE:
      Serial.println("\n>> WiFi scan DONE!");
      break;

    case ARDUINO_EVENT_WIFI_STA_START:
      Serial.println("\n>> WiFi STA start");
      break;

    case ARDUINO_EVENT_WIFI_STA_GOT_IP:
      Serial.println("\n>> WiFi connected");
      break;

    case ARDUINO_EVENT_WIFI_STA_LOST_IP:
      Serial.println("\n>> WiFi lost IP?");
      /* fall through */
    case ARDUINO_EVENT_WIFI_STA_DISCONNECTED:
      Serial.println("\n>> WiFi lost connection");
      break;

    case ARDUINO_EVENT_WIFI_AP_STACONNECTED:
      Serial.println("\n>> WiFi AP_STA connected");
      break;

    case ARDUINO_EVENT_WIFI_AP_STADISCONNECTED:
      Serial.println("\n>> WiFi AP_STA disconnected");
      break;

    case ARDUINO_EVENT_WIFI_AP_STAIPASSIGNED:
      Serial.println("\n>> WiFi AP_STA assigned");
      break;

    default:
      Serial.printf("\n>> WiFi evt: %d\n", (int)(event->event_id));
      break;
  }
}

bool should_start_a_scan = false;

void open_portal() {
  if (WiFi.getMode() & WIFI_MODE_AP) return;

  WiFi.softAP("test-ap");
  init_dnsServer();
}

bool should_close_portal = false;

void close_portal() {
  const auto cur_mode = WiFi.getMode();
  WiFi.mode((wifi_mode_t)(cur_mode & ~WIFI_MODE_AP));  // close AP..

  WiFi.begin();  // just to be sure..
}

void setup() {
  Serial.begin(115200);

  WiFi.setHostname("slaesh-iwr-308398d1a644");
  WiFi.begin();

  delay(4000);

  WiFi.setSleep(false);
  Serial.printf("WiFi.sleep is enabled? %d\n", WiFi.getSleep());
  Serial.printf("WiFi.TxPower is? %d\n", WiFi.getTxPower());

  storage_init();
  wcs_init();
  cps_init(custom_params, n_custom_params, true);

  const String use_led = cps_get_param(WM_CP_USE_LED_KEY);
  Serial.printf("we should %suse the led!\n", use_led == "yes" ? "" : "NOT ");

  WiFi.onEvent(_wifiEvents);

  if (wcs_stored_credentials_count() == 0) {
    // no need to enable STA.. yet!
    open_portal();
  }

  DefaultHeaders::Instance().addHeader("access-control-allow-origin", "*");
  DefaultHeaders::Instance().addHeader("access-control-allow-methods",
                                       "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("access-control-allow-headers", "*");

  asyncWebserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (captivePortal(request)) {
      return;
    }

    String idxHtml = INDEX_HTML;
    idxHtml.replace("{{test}}", "DU");

    request->send(200, HTTP_CONTENT_TYPE_HTML, idxHtml.c_str());
  });

  asyncWebserver.on("/close", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);

    // no creds stored yet? leave it open no matter what! =)
    if (wcs_stored_credentials_count() > 0) {
      should_close_portal = true;
    }
  });

  asyncWebserver.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);

    ESP.restart();
  });

  wifi_routes_add(asyncWebserver);
  param_routes_add(asyncWebserver);

  asyncWebserver.onNotFound([](AsyncWebServerRequest *request) {
    if (captivePortal(request)) {
      return;
    }

    request->send(200, HTTP_CONTENT_TYPE_TEXT, "404");
  });

  asyncWebserver.begin();
}

void check_scan_results() {
  // not started?
  if (start_wifi_scan == 0) return;

  delay(10);  // we need a small delay to get the scan kick in.. otherwise the
              // status will be "failed"..

  const auto scan_status = WiFi.scanComplete();
  // still running?
  if (scan_status == WIFI_SCAN_RUNNING) return;

  const auto scan_done = millis();
  Serial.printf("scan %s.. took %d ms\n",
                scan_status == WIFI_SCAN_FAILED ? "FAILED" : "DONE",
                scan_done - start_wifi_scan);
  start_wifi_scan = 0;

  // failed?
  if (scan_status == WIFI_SCAN_FAILED) return;

  // not running, nor failed.. we did it! :)
  int16_t n_networks = scan_status;
  Serial.printf("networks found %d\n", n_networks);
  if (n_networks <= 0) return;

  for (uint16_t i = 0; i < n_networks; ++i) {
    // Print SSID and RSSI for each network found
    Serial.printf("%02d", i + 1);
    Serial.print(": ");
    Serial.print(WiFi.SSID(i));
    Serial.print(" (");
    Serial.print(WiFi.RSSI(i));
    Serial.print(")");
    Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " " : "*");
    delay(2);
  }

  Serial.println("");
}

void check_wifi_status() {
#define CHECK_WIFI_INTERVAL            (2000)
#define WIFI_RECONNECT_THRESHOLD       (20000)
#define WIFI_RECONNECT_WAITING_TIMEOUT (15000)

  static uint32_t reconnect_count  = 0;
  static uint32_t disconnect_count = 0;
  static TickTimer timer(CHECK_WIFI_INTERVAL);

  const auto now = millis();
  if (timer.isTicking(now)) {
    return;
  }
  timer.reset(now);

  // everything is fine?
  if (WiFi.status() == WL_CONNECTED) {
    disconnect_count = 0;
    reconnect_count  = 0;

    // auto reconnect off? but we are connected..
    if (!WiFi.getAutoReconnect()) {
      // .. enable it!
      // so if we loose the connection, WiFi will try to reconnect itself
      // .. if we are longer disconnected, our re-connect mechanism will kick in
      WiFi.setAutoReconnect(true);
    }

    return;
  };

  // .. seems like we are disconnected.. =/
  // wait some time.. maybe the auto-reconnect will get us back on!
  if (disconnect_count < WIFI_RECONNECT_THRESHOLD / CHECK_WIFI_INTERVAL) {
    ++disconnect_count;
    Serial.printf("tick.. tack.. %d\n", disconnect_count);

    return;
  }

  const auto n_wifi_creds = wcs_stored_credentials_count();
  // no creds stored? we cant do anything :(
  if (n_wifi_creds == 0) {
    Serial.printf("we have no creds anyway..\n");

    return;
  }

  // offline and creds stored..?
  // turn off the auto-reconnect thingy.. this will collide with a scan probably
  // we need to try it manually.. so disable auto reconnect!
  WiFi.setAutoReconnect(false);

  Serial.printf("WiFi disconnected.. mode %d\n", WiFi.getMode());

  // currently scanning? do not interrupt that! :)
  if (should_start_a_scan || start_wifi_scan) {
    Serial.printf(".. about to SCAN, leave for now..\n");

    return;
  }

  static uint8_t wifi_cred_idx = 0;
  static TickTimer reconnectingTimer(WIFI_RECONNECT_WAITING_TIMEOUT,
                                     now - WIFI_RECONNECT_WAITING_TIMEOUT - 1);

  // we will iterate over all stored creds.. giving each time to connect though!
  if (reconnectingTimer.isTicking(now)) {
    // Serial.printf("\tstill connecting..\n");

    return;
  }

  const auto c = wifi_credentials + wifi_cred_idx;

  Serial.printf("\tconnecting to(%d) '%s' :: '%s'\n", wifi_cred_idx, c->ssid,
                c->pass);

  WiFi.begin(c->ssid, c->pass);

  // reset timer
  reconnectingTimer.reset(now);

  // .. and go to the next one..
  wifi_cred_idx = (wifi_cred_idx + 1) % n_wifi_creds;

  // we got through all and are not connected yet?
  if (wifi_cred_idx == 0 && WiFi.status() != WL_CONNECTED &&
      !(WiFi.getMode() & WIFI_MODE_AP) && reconnect_count > 0) {
    Serial.println("open AP!!");

    open_portal();
  }

  ++reconnect_count;
}

void loop() {
  check_scan_results();

  if (should_close_portal) {
    should_close_portal = false;
    close_portal();
  }

  if (should_start_a_scan) {
    should_start_a_scan = false;
    start_a_scan();
  }

  check_wifi_status();

  while (Serial.available()) {
    const auto b = Serial.read();
    if (b == 'x') {
      open_portal();
    }
  }

  delay(1);
}

// TODO check param_length_test_Screenshot_2022-11-10_18-44-24
// --> mqtt server abgeschnitten?
// post:
/*
[
  [
    "frnd_n",
    "test office mit ganz langem namen 123456"
  ],
  [
    "mqtt_s",
    "mqtt.slae.shnochtviellängergehtwohlkaum1"
  ],
  [
    "mqtt_po",
    "3881"
  ],
  [
    "mqtt_u",
    "supercoolerusername1"
  ],
  [
    "mqtt_pa",
    "dummywaseinpass12345"
  ],
  [
    "ota_s",
    "srv.slae.shfillmeupbabyboyindaface123456"
  ],
  [
    "ota_p",
    "35982"
  ],
  [
    "ota_u",
    "/fw/update/und/noch/viel/mehr/123456789a"
  ],
  [
    "ota_c",
    "dev"
  ]
]
*/

// !!!! UMLAUTE ??????
// saving: 'mqtt.slae.shnochtviellängergehtwohlkaum1'
// saved:  'mqtt.slae.shnochtviellängergehtwohlkaum' ???!
