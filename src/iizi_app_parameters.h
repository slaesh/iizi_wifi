#ifndef __IIZI_APP_PARAMETERS_H__
#define __IIZI_APP_PARAMETERS_H__

#include "stdint.h"

typedef struct {
  const char *key;
  const char *label;
  const char *value;
  const char *type;
  const uint16_t max_length;
} t_iizi_app_parameter;

void iizi_app_parameters(t_iizi_app_parameter *params, const uint16_t n_params,
                         bool read_config);
const char *iizi_get_parameter_value(const char *param_key);

#endif  // __IIZI_APP_PARAMETERS_H__
