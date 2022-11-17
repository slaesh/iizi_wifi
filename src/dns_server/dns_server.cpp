
#define __DNS_SERVER_C__

#include "dns_server.h"

#include "WiFi.h"

AsyncDNSServer dnsServer;

bool init_dns_server() {
  dnsServer.stop();

  dnsServer.setErrorReplyCode(AsyncDNSReplyCode::NoError);

  // every request should be redirected to ourselves! ;)
  if (dnsServer.start(53, "*", WiFi.softAPIP())) {
    return true;
  }

  // Serial.println("CANT START DNS SERVER!!");

  return false;
}
