#define __IIZI_WIFI_MNG_C__

#include "iizi_wifi.h"

#include "ESPAsyncDNSServer.h"
#include "ESPAsyncWebServer.h"
#include "dns_server/dns_server.h"
#include "iizi_portal.h"
#include "iizi_task.h"
#include "storage/custom_param_storage.h"
#include "storage/storage_init.h"
#include "storage/wifi_credential_storage.h"


void iizi_wifi_mng_init(String hostname) {
  // set hostname, needs to be done BEFORE we init the WiFi itself!
  iizi_portal_set_hostname(hostname);

  WiFi.begin();

  WiFi.setSleep(false);
  Serial.printf("WiFi.sleep is enabled? %d\n", WiFi.getSleep());
  Serial.printf("WiFi.TxPower is? %d\n", WiFi.getTxPower());

  storage_init();
  wcs_init();

  iizi_create_task();
}

void iizi_app_parameters(t_iizi_app_parameter *params, const uint16_t n_params,
                         boolean read_config) {
  cps_init(params, n_params, read_config);
}
