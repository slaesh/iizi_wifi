#define __CUSTOM_PARAM_STORAGE_C__

#include "custom_param_storage.h"

#include <LittleFS.h>

#include "FS.h"

#define FileFS   LittleFS
#define CPS_FILE F("/__iizi.cps")

static t_iizi_app_parameter *user_parameters = NULL;
static uint16_t n_custom_params              = 0;

uint16_t cps_custom_parameter_count() { return n_custom_params; }

void cps_init(t_iizi_app_parameter *params, const uint16_t n_params,
              boolean read_config) {
  user_parameters = params;
  n_custom_params = n_params;

  if (read_config) {
    cps_read();
  }
}

uint32_t cps_calculate_key_value_json_array_size(
    // only needed for deserialization
    boolean including_memory_for_values) {
  if (user_parameters == NULL || n_custom_params == 0) return 0;

  const auto base_size =
      // parent array
      JSON_ARRAY_SIZE(n_custom_params) +
      // nested arrays (key, value)
      JSON_ARRAY_SIZE(2) * n_custom_params;

  if (!including_memory_for_values) {
    return base_size;
  }

  // TODO: calculate the actual value..
  const auto mem_for_key_and_values = n_custom_params * 80;

  return base_size + mem_for_key_and_values;
}

uint32_t cps_calculate_full_json_array_size() {
  if (user_parameters == NULL || n_custom_params == 0) return 0;

  return
      // parent array
      JSON_ARRAY_SIZE(n_custom_params) +
      // nested arrays (key, value, type, label, max_len, group)
      JSON_ARRAY_SIZE(6) * n_custom_params +
      // max_len will be copied, rest should be only pointers!
      n_custom_params * sizeof(uint16_t);
}

boolean cps_to_json_array(const JsonArray &json_array) {
  // nothing to do
  if (user_parameters == NULL || n_custom_params == 0) return true;

  for (uint16_t i = 0; i < n_custom_params; ++i) {
    const auto p = user_parameters[i];
    const auto a = json_array.createNestedArray();
    // order of entries needs to be [key, val, type, max_length, label, group]
    a.add(p.key);
    a.add(p.value);
    a.add(p.type);
    a.add(p.max_length);
    a.add(p.label);
  }

  return true;
}

boolean cps_save() {
  File f = FileFS.open(CPS_FILE, "w", true);
  if (!f) {
    Serial.println("cps_save(): cant open file??");
    return false;
  }

  const auto json_doc_size = cps_calculate_key_value_json_array_size(false);

  Serial.printf("cps_save(): json_doc_size: %d\n", json_doc_size);

  DynamicJsonDocument json(json_doc_size);
  const auto params_array = json.to<JsonArray>();

  for (uint16_t i = 0; i < n_custom_params; ++i) {
    const auto p = user_parameters[i];
    const auto a = params_array.createNestedArray();
    a.add(p.key);
    a.add(p.value);
  }

  const auto n_bytes_written = serializeJson(json, f);

  Serial.printf("cps_save(): %d bytes written\n", n_bytes_written);

  f.flush();
  f.close();

  return true;
}

boolean cps_read_from_array(const JsonArray &custom_params_key_values) {
  if (custom_params_key_values.size() == 0) return true;

  for (auto key_value : custom_params_key_values) {
    if (key_value.size() != 2) continue;

    const auto key = key_value[0].as<String>();
    const auto val = key_value[1].as<String>();

    Serial.printf("cps_read_from_array(): k(%s) :: v(%s)\n", key.c_str(),
                  val.c_str());

    for (uint16_t i = 0; i < n_custom_params; ++i) {
      const auto p = user_parameters + i;
      if (p == NULL) continue;
      if (key != String(p->key)) continue;

      char *cur_val = (char *)(p->value);

      strncpy(cur_val, val.c_str(), p->max_length + 1);

      // take care of termination!
      if (cur_val[p->max_length] != 0) {
        cur_val[p->max_length] = 0;
      }
    }
  }

  return true;
}

boolean cps_read() {
  File f = FileFS.open(CPS_FILE, "r");
  if (!f) {
    Serial.println("cps_read(): cant open file??");
    return false;
  }

  const auto json_doc_size = cps_calculate_key_value_json_array_size(true);

  Serial.printf("cps_read(): json_doc_size: %d\n", json_doc_size);

  DynamicJsonDocument json(json_doc_size);

  // ReadBufferingStream bufferingStream(f, 64);
  // const auto deserialize_error = deserializeJson(creds, bufferingStream);

  const auto deserialize_error = deserializeJson(json, f);
  if (deserialize_error) {
    Serial.print(F("cps_read(): deserializeJson() failed with code "));
    Serial.println(deserialize_error.f_str());

    return false;
  }

  const auto custom_params_key_values = json.as<JsonArray>();
  return cps_read_from_array(custom_params_key_values);
}

const char *iizi_get_parameter_value(const char *param_key) {
  String key = param_key;

  for (uint16_t i = 0; i < n_custom_params; ++i) {
    const auto p = user_parameters + i;
    if (p == NULL) continue;
    if (key != String(p->key)) continue;

    return p->value;
  }

  return NULL;
}
