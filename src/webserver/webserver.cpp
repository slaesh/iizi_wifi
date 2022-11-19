
#include "ESPAsyncWebServer.h"
#include "html/html.h"
#include "param_routes.h"
#include "wifi_routes.h"

static AsyncWebServer asyncWebserver(80);

boolean isIp(const String &str) {
  for (unsigned int i = 0; i < str.length(); i++) {
    int c = str.charAt(i);

    if ((c != '.') && (c != ':') && ((c < '0') || (c > '9'))) {
      return false;
    }
  }

  return true;
}

String toStringIp(const IPAddress &ip) {
  String res = "";

  for (int i = 0; i < 3; i++) {
    res += String((ip >> (8 * i)) & 0xFF) + ".";
  }

  res += String(((ip >> 8 * 3)) & 0xFF);

  return res;
}

boolean already_redirected(AsyncWebServerRequest *request) {
  Serial.printf("request host IP = %s\n", request->host().c_str());

  // TODO: include it from somewhere?
  extern char *iizi_portal_hostname;
  Serial.printf("ourHostname1 = '%s'\n", iizi_portal_hostname);

  String ourHostname = iizi_portal_hostname;
  Serial.printf("ourHostname2 = '%s'\n", ourHostname.c_str());

  // our hostname? thats fine!
  if (request->host() == ourHostname) return false;
  // our ip? thats fine too!
  if (isIp(request->host()) &&
      toStringIp(request->client()->localIP()) == request->host())
    return false;

  // .. otherwise redirect!
  String redirectRoute =
      String("http://") + toStringIp(request->client()->localIP());
  Serial.printf("redirecting to our root route! '%s'\n", redirectRoute.c_str());

  request->redirect(redirectRoute);

  return true;
}

void webserver_init() {
  asyncWebserver.reset();

  DefaultHeaders::Instance().addHeader("access-control-allow-origin", "*");
  DefaultHeaders::Instance().addHeader("access-control-allow-methods",
                                       "GET, POST, PUT");
  DefaultHeaders::Instance().addHeader("access-control-allow-headers", "*");

  asyncWebserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (already_redirected(request)) {
      return;
    }

    String idxHtml = INDEX_HTML;
    idxHtml.replace("{{test}}", "DU");

    request->send(200, HTTP_CONTENT_TYPE_HTML, idxHtml.c_str());
  });

  asyncWebserver.on("/close", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);

    // TODO: include it from somewhere?
    extern bool iizi_portal_should_close;
    iizi_portal_should_close = true;
  });

  asyncWebserver.on("/reset", HTTP_POST, [](AsyncWebServerRequest *request) {
    request->send(200);

    ESP.restart();
  });

  wifi_routes_add(asyncWebserver);
  param_routes_add(asyncWebserver);

  asyncWebserver.onNotFound([](AsyncWebServerRequest *request) {
    if (already_redirected(request)) {
      return;
    }

    request->send(200, HTTP_CONTENT_TYPE_TEXT, "404");
  });
}

void webserver_start() { asyncWebserver.begin(); }
