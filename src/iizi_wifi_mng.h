#ifndef __IIZI_WIFI_MNG_H__
#define __IIZI_WIFI_MNG_H__

#include <Arduino.h>

// TODO: include only the bare minimum!!
#include "storage/custom_param_storage.h"

void iizi_wifi_mng_init(String hostname);

// storage functions

void iizi_app_parameters(t_iizi_app_parameter *params, const uint16_t n_params,
                         boolean read_config);
const char *iizi_get_parameter_value(const char *param_key);

#endif  // __IIZI_WIFI_MNG_H__
