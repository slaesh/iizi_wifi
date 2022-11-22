#ifndef __IIZI_PORTAL_H__
#define __IIZI_PORTAL_H__

#include "WString.h"
#include "stdint.h"

void iizi_portal_set_hostname(String hostname);
const char *iizi_portal_get_hostname();
void iizi_portal_open(uint32_t timeout = 0);
void iizi_portal_close();
bool iizi_portal_is_open();
uint8_t iizi_portal_clients_connected();

#endif  // __IIZI_PORTAL_H__
