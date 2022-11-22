#define __IIZI_WIFI_C__

#include "iizi_wifi.h"

#include "Arduino.h"
#include "WiFi.h"
#include "is_timer_expired.h"
#include "stdint.h"
#include "storage/wifi_credential_storage.h"

uint32_t iizi_wifi_scan_started = 0;

void iizi_wifi_start_a_scan() {
  iizi_wifi_scan_started = millis();
  if (iizi_wifi_scan_started == 0) ++iizi_wifi_scan_started;

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

void iizi_wifi_check_scan_results() {
  // not started?
  if (iizi_wifi_scan_started == 0) return;

  delay(10);  // we need a small delay to get the scan kick in.. otherwise the
              // status will be "failed"..

  const auto scan_status = WiFi.scanComplete();
  // still running?
  if (scan_status == WIFI_SCAN_RUNNING) return;

  const auto scan_done = millis();
  Serial.printf("scan %s.. took %d ms\n",
                scan_status == WIFI_SCAN_FAILED ? "FAILED" : "DONE",
                scan_done - iizi_wifi_scan_started);
  iizi_wifi_scan_started = 0;

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

void iizi_wifi_check_status() {
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
  extern bool iizi_wifi_should_start_scan;
  if (iizi_wifi_should_start_scan || iizi_wifi_scan_started) {
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

    extern bool iizi_portal_should_open;
    iizi_portal_should_open = true;
  }

  ++reconnect_count;
}

void iizi_wifi_loop() {
  iizi_wifi_check_scan_results();
  iizi_wifi_check_status();
}
