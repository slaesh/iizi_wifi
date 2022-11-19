
#include "../portal/iizi_portal.h"
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

  const auto iizi_portal_hostname = iizi_portal_get_hostname();
  String ourHostname              = iizi_portal_hostname;

  // our hostname? thats fine!
  if (request->host() == ourHostname) return false;
  // our ip? thats fine too!
  if (isIp(request->host()) &&
      toStringIp(request->client()->localIP()) == request->host())
    return false;

  // .. otherwise redirect!
  String apIp          = toStringIp(request->client()->localIP());
  String redirectRoute = String("http://") + apIp;

  Serial.printf("redirecting to our root route! '%s'\n", redirectRoute.c_str());

  request->redirect(redirectRoute);

  return true;
}

AsyncWebServer *webserver_init() {
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
    idxHtml.replace("{{hostname}}", iizi_portal_get_hostname());

    String wifi_info             = "not connected";
    const auto is_wifi_connected = WiFi.status() == WL_CONNECTED;
    if (is_wifi_connected) {
      wifi_info = "connected to <b>" + WiFi.SSID() + "</b>";
    }

    idxHtml.replace("{{wifi}}", wifi_info);
    idxHtml.replace("{{ip}}", WiFi.localIP().toString().c_str());
    idxHtml.replace("{{close_ap_btn_disabled}}",
                    WiFi.getMode() & WIFI_MODE_AP ? "" : "disabled");

    request->send(200, HTTP_CONTENT_TYPE_HTML, idxHtml);
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

  return &asyncWebserver;
}

AsyncWebServer *webserver_instance() { return &asyncWebserver; }
