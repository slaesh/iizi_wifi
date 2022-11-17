#ifndef __PARAMETERS_H__
#define __PARAMETERS_H__

#include "../storage/custom_param_storage.h"

/*
xxx_KEY's need to be unique !
xxx_LEN is WITHOUT the null terminition
*/

#define WM_CP_FRIENDLY_NAME_KEY        ("frnd_n")
#define WM_CP_FRIENDLY_NAME_LEN        (40)
#define WM_CP_USE_LED_KEY              ("dev_led")
#define WM_CP_USE_LED_LEN              (3)  // yes / no
#define WM_CP_INPUT_TOGGLES_OUTPUT_KEY ("dev_ito")
#define WM_CP_INPUT_TOGGLES_OUTPUT_LEN (3)  // yes / no

#define WM_CP_MQTT_HOST_KEY            ("mqtt_s")
#define WM_CP_MQTT_HOST_LEN            (40)
#define WM_CP_MQTT_PORT_KEY            ("mqtt_po")
#define WM_CP_MQTT_PORT_LEN            (5)
#define WM_CP_MQTT_USER_KEY            ("mqtt_u")
#define WM_CP_MQTT_USER_LEN            (20)
#define WM_CP_MQTT_PASS_KEY            ("mqtt_pa")
#define WM_CP_MQTT_PASS_LEN            (20)

#define WM_CP_OTA_UPDATE_HOST_KEY      ("ota_s")
#define WM_CP_OTA_UPDATE_HOST_LEN      (40)
#define WM_CP_OTA_UPDATE_PORT_KEY      ("ota_p")
#define WM_CP_OTA_UPDATE_PORT_LEN      (5)
#define WM_CP_OTA_UPDATE_PATH_KEY      ("ota_u")
#define WM_CP_OTA_UPDATE_PATH_LEN      (40)
#define WM_CP_OTA_UPDATE_CHANNEL_KEY   ("ota_c")
#define WM_CP_OTA_UPDATE_CHANNEL_LEN   (6)

extern t_cps_parameter custom_params[];
extern const uint16_t n_custom_params;

#endif  // __PARAMETERS_H__
