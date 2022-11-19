
#include "WiFi.h"
#include "dns_server/dns_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage/wifi_credential_storage.h"
#include "webserver/webserver.h"

static String ap_name;

char iizi_portal_hostname[32 + 1] = "TODO_USE_MAC";

void iizi_portal_set_hostname(String hostname) {
  if (hostname == "") {
    // TODO: set with MAC addr..
  }

  Serial.printf("copy hostname '%s', max len: %d\n", hostname.c_str(),
                sizeof(iizi_portal_hostname));

  // copy desired hostname
  // strncpy(iizi_portal_hostname, hostname.c_str(),
  // sizeof(iizi_portal_hostname));
  strncpy(iizi_portal_hostname, hostname.c_str(), sizeof(iizi_portal_hostname));
  iizi_portal_hostname[sizeof(iizi_portal_hostname)] = 0;

  Serial.printf("copied hostname '%s', str len: %d\n", iizi_portal_hostname,
                strlen(iizi_portal_hostname));

  WiFi.setHostname(iizi_portal_hostname);
}

void iizi_portal_open(uint32_t timeout) {
  // AP already opened?
  if (WiFi.getMode() & WIFI_MODE_AP) return;

  ap_name = "IIZI_";
  ap_name += iizi_portal_hostname;

  // set AP name
  WiFi.softAP(ap_name.c_str());

  // set up our dns server
  init_dns_server();

  // set up our webserver
  webserver_init();

  // .. and start it right away
  webserver_start();
}

void iizi_portal_close() {
  const auto cur_wifi_mode = WiFi.getMode();
  WiFi.mode((wifi_mode_t)(cur_wifi_mode & ~WIFI_MODE_AP));  // close AP..

  WiFi.begin();  // just to be sure..
}
