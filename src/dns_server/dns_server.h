#ifndef __DNS_SERVER_H__
#define __DNS_SERVER_H__

#include "ESPAsyncDNSServer.h"

extern AsyncDNSServer dnsServer;

bool init_dns_server();

#endif  // __DNS_SERVER_H__
