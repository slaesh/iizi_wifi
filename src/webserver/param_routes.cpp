
#define __PARAM_ROUTES_C__

#include "param_routes.h"

#include <ArduinoJson.h>

#include "../storage/custom_param_storage.h"
#include "AsyncJson.h"
#include "html/html.h"
#include "webserver_config.h"

void GET_params_page(AsyncWebServerRequest *request) {
  String paramsHtml = PARAMS_HTML;

  // TODO: replace something.. otherwise, pass in the PARAMS_HTML directly!!

  request->send(200, HTTP_CONTENT_TYPE_HTML, paramsHtml);
}

void GET_params_query_handler(AsyncWebServerRequest *request) {
  const auto json_size = cps_calculate_full_json_array_size();

  Serial.printf("/params-query :: json_size: %d\n", json_size);

  DynamicJsonDocument json(json_size +
                           JSON_OBJECT_SIZE(2 /* params, n_params */));

  json["n_params"]  = cps_custom_parameter_count();
  const auto params = json.createNestedArray("params");
  cps_to_json_array(params);

  const auto response = request->beginResponseStream(HTTP_CONTENT_TYPE_JSON);
  serializeJson(json, *response);
  request->send(response);
}

static auto *POST_params_save_handler = new AsyncCallbackJsonWebHandler(
    EAF_WM_WEBSERVER_PREFIX "/params-save",
    [](AsyncWebServerRequest *request, JsonVariant &json) {
      const auto params = json["params"].as<JsonArray>();
      if (params.isNull()) {
        return request->send(200);
      }

      cps_read_from_array(params);
      cps_save();

      request->send(200);
    });

void param_routes_add(AsyncWebServer &webserver) {
  webserver.on(EAF_WM_WEBSERVER_PREFIX "/params", HTTP_GET, GET_params_page);

  webserver.on(EAF_WM_WEBSERVER_PREFIX "/params-query", HTTP_GET,
               GET_params_query_handler);

  POST_params_save_handler->setMethod(HTTP_POST);
  webserver.addHandler(POST_params_save_handler);
}
