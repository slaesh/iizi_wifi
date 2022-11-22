
#include "WiFi.h"
#include "dns_server/dns_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage/wifi_credential_storage.h"
#include "webserver/webserver.h"

static String ap_name;
static char iizi_portal_hostname[32 + 1] = "";

void iizi_portal_set_hostname(String hostname) {
  if (hostname == "") {
    hostname = "iizi_" + WiFi.macAddress();
    hostname.replace(":", "");
    hostname.toLowerCase();
  }

  // copy desired hostname
  strncpy(iizi_portal_hostname, hostname.c_str(), sizeof(iizi_portal_hostname));
  iizi_portal_hostname[sizeof(iizi_portal_hostname)] = 0;

  WiFi.setHostname(iizi_portal_hostname);
}

const char *iizi_portal_get_hostname() { return iizi_portal_hostname; }

bool iizi_portal_is_open() { return WiFi.getMode() & WIFI_MODE_AP; }

void iizi_portal_open(uint32_t timeout) {
  if (iizi_portal_is_open()) return;

  ap_name = "iizi_";
  ap_name += iizi_portal_hostname;

  // set AP name
  WiFi.softAP(ap_name.c_str());

  // set up our dns server
  dns_server_init();

  // start the webserver
  const auto webserver = webserver_instance();
  webserver->begin();
}

void iizi_portal_close() {
  dns_server_stop();

  WiFi.enableAP(false);  // close AP..

  if (WiFi.getMode() & WIFI_MODE_STA) return;

  WiFi.begin();
}

uint8_t iizi_portal_clients_connected() { return WiFi.softAPgetStationNum(); }
