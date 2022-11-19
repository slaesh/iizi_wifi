#ifndef __IIZI_PORTAL_H__
#define __IIZI_PORTAL_H__

#include "WString.h"
#include "stdint.h"

void iizi_portal_set_hostname(String hostname);
void iizi_portal_open(uint32_t timeout = 0);
void iizi_portal_close();

#endif  // __IIZI_PORTAL_H__
