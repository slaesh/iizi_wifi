#ifndef __IIZI_WIFI_MNG_H__
#define __IIZI_WIFI_MNG_H__

#include <Arduino.h>

void iizi_wifi_mng_init(String hostname);
void iizi_wifi_mng_open_portal(uint32_t timeout = 0);

#endif  // __IIZI_WIFI_MNG_H__
