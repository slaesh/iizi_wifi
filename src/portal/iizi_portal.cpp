
#include "WiFi.h"
#include "dns_server/dns_server.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "storage/wifi_credential_storage.h"
#include "webserver/webserver.h"

static String ap_name;
static char iizi_portal_hostname[32 + 1] = "TODO_USE_MAC";

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

void iizi_portal_open(uint32_t timeout) {
  // AP already opened?
  if (WiFi.getMode() & WIFI_MODE_AP) return;

  ap_name = "iizi_";
  ap_name += iizi_portal_hostname;

  // set AP name
  WiFi.softAP(ap_name.c_str());

  // set up our dns server
  dns_server_init();

  // TODO: trust that the webserver is running? or maybe start it again?
  // --> check if we "just can start it again" without creating memory
  // leaks?????
}

void iizi_portal_close() {
  dns_server_stop();

  const auto cur_wifi_mode = WiFi.getMode();
  WiFi.mode((wifi_mode_t)(cur_wifi_mode & ~WIFI_MODE_AP));  // close AP..

  WiFi.begin();  // just to be sure..
}
