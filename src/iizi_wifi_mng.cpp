#define __IIZI_WIFI_MNG_C__

#include "iizi_wifi_mng.h"

#include "ESPAsyncDNSServer.h"
#include "ESPAsyncWebServer.h"
#include "dns_server/dns_server.h"
#include "portal/iizi_portal.h"
#include "storage/custom_param_storage.h"
#include "storage/storage_init.h"
#include "storage/wifi_credential_storage.h"
#include "task/iizi_task.h"
#include "webserver/webserver.h"
#include "wifi/iizi_wifi.h"

void iizi_wifi_mng_init(String hostname, bool start_webserver) {
  // set hostname, needs to be done BEFORE we init the WiFi itself!
  iizi_portal_set_hostname(hostname);

  WiFi.begin();
  WiFi.setSleep(false);

  storage_init();
  wcs_init();

  // set up our web server
  auto webserver = webserver_init();
  if (start_webserver) {
    webserver.begin();
  }

  iizi_create_task();
}

void iizi_app_parameters(t_iizi_app_parameter *params, const uint16_t n_params,
                         boolean read_config) {
  cps_init(params, n_params, read_config);
}

void iizi_wifi_mng_open_portal(uint32_t timeout) {
  // TODO: make use of the timeout .. !!
  extern bool iizi_portal_should_open;
  iizi_portal_should_open = true;
}

AsyncWebServer &iizi_wifi_mng_webserver_instance() {
  return webserver_instance();
}
