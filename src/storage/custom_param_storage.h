#ifndef __CUSTOM_PARAM_STORAGE_H__
#define __CUSTOM_PARAM_STORAGE_H__

#include <ArduinoJson.h>

typedef struct {
  const char *key;
  const char *label;
  const char *value;
  const char *type;
  const uint16_t max_length;
} t_iizi_app_parameter;

uint16_t cps_custom_parameter_count();

void cps_init(t_iizi_app_parameter *params, const uint16_t n_custom_params,
              boolean read_config = true);

uint32_t cps_calculate_key_value_json_array_size(
    boolean including_memory_for_values);
uint32_t cps_calculate_full_json_array_size();
boolean cps_to_json_array(const JsonArray &json_array);
boolean cps_save();
boolean cps_read_from_array(const JsonArray &custom_params_key_values);
boolean cps_read();
const char *iizi_get_parameter_value(const char *param_key);

#endif  // __CUSTOM_PARAM_STORAGE_H__
