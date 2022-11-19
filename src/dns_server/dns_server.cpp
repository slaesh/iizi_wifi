
#define __DNS_SERVER_C__

#include "dns_server.h"

#include "ESPAsyncDNSServer.h"
#include "WiFi.h"

static AsyncDNSServer dnsServer;

bool dns_server_init() {
  dnsServer.stop();

  dnsServer.setErrorReplyCode(AsyncDNSReplyCode::NoError);

  // every request should be redirected to ourselves! ;)
  if (dnsServer.start(53, "*", WiFi.softAPIP())) {
    return true;
  }

  // Serial.println("CANT START DNS SERVER!!");

  return false;
}

void dns_server_stop() { dnsServer.stop(); }
