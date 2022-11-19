#ifndef __IIZI_WIFI_MNG_H__
#define __IIZI_WIFI_MNG_H__

#include "ESPAsyncWebServer.h"
#include "WString.h"
#include "stdint.h"

void iizi_wifi_mng_init(String hostname, bool start_webserver = true);
void iizi_wifi_mng_open_portal(uint32_t timeout = 0);
AsyncWebServer *iizi_wifi_mng_webserver_instance();

#endif  // __IIZI_WIFI_MNG_H__
