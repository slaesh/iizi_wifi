#define __PARAMETERS_C__

#include "parameters.h"

static char cps_friendly_name[WM_CP_FRIENDLY_NAME_LEN + 1] = "";
static char cps_use_led[WM_CP_USE_LED_LEN + 1]             = "yes";
static char cps_input_toggles_output[WM_CP_INPUT_TOGGLES_OUTPUT_LEN + 1] =
    "yes";

static char cps_mqtt_host[WM_CP_MQTT_HOST_LEN + 1] = "mqtt.slae.sh";
static char cps_mqtt_port[WM_CP_MQTT_PORT_LEN + 1] = "3881";
static char cps_mqtt_user[WM_CP_MQTT_USER_LEN + 1] = "iwr";
static char cps_mqtt_pass[WM_CP_MQTT_PASS_LEN + 1] = "";

static char cps_ota_host[WM_CP_OTA_UPDATE_HOST_LEN + 1]    = "srv.slae.sh";
static char cps_ota_port[WM_CP_OTA_UPDATE_PORT_LEN + 1]    = "35982";
static char cps_ota_path[WM_CP_OTA_UPDATE_PATH_LEN + 1]    = "/fw/update";
static char cps_ota_chan[WM_CP_OTA_UPDATE_CHANNEL_LEN + 1] = "stable";

t_cps_parameter custom_params[] = {

#if true  // DEVICE

    {
        .key        = WM_CP_FRIENDLY_NAME_KEY,
        .label      = "friendly name",
        .value      = cps_friendly_name,
        .type       = "str",
        .max_length = WM_CP_FRIENDLY_NAME_LEN,
    },

    {
        .key        = WM_CP_USE_LED_KEY,
        .label      = "blinky led",
        .value      = cps_use_led,
        .type       = "enum|yes|no",
        .max_length = WM_CP_USE_LED_LEN,
    },

    {
        .key        = WM_CP_INPUT_TOGGLES_OUTPUT_KEY,
        .label      = "input toggles output",
        .value      = cps_input_toggles_output,
        .type       = "enum|yes|no",
        .max_length = WM_CP_INPUT_TOGGLES_OUTPUT_LEN,
    },

#endif

#if true  // MQTT

    {
        .key        = WM_CP_MQTT_HOST_KEY,
        .label      = "mqtt server",
        .value      = cps_mqtt_host,
        .type       = "str",
        .max_length = WM_CP_MQTT_HOST_LEN,
    },

    {
        .key        = WM_CP_MQTT_PORT_KEY,
        .label      = "mqtt port",
        .value      = cps_mqtt_port,  // TODO: change default back to 1883
        .type       = "int",
        .max_length = WM_CP_MQTT_PORT_LEN,
    },

    {
        .key        = WM_CP_MQTT_USER_KEY,
        .label      = "mqtt user",
        .value      = cps_mqtt_user,
        .type       = "str",
        .max_length = WM_CP_MQTT_USER_LEN,
    },

    {
        .key        = WM_CP_MQTT_PASS_KEY,
        .label      = "mqtt pass",
        .value      = cps_mqtt_pass,
        .type       = "str",
        .max_length = WM_CP_MQTT_PASS_LEN,
    },

#endif

#if true  // OTA UPDATES

    {
        .key        = WM_CP_OTA_UPDATE_HOST_KEY,
        .label      = "ota update server",
        .value      = cps_ota_host,
        .type       = "str",
        .max_length = WM_CP_OTA_UPDATE_HOST_LEN,
    },

    {
        .key        = WM_CP_OTA_UPDATE_PORT_KEY,
        .label      = "ota update port",
        .value      = cps_ota_port,
        .type       = "int",
        .max_length = WM_CP_OTA_UPDATE_PORT_LEN,
    },

    {
        .key        = WM_CP_OTA_UPDATE_PATH_KEY,
        .label      = "ota update url",
        .value      = cps_ota_path,
        .type       = "str",
        .max_length = WM_CP_OTA_UPDATE_PATH_LEN,
    },

    {
        .key        = WM_CP_OTA_UPDATE_CHANNEL_KEY,
        .label      = "ota update channel",
        .value      = cps_ota_chan,
        .type       = "enum|dev|beta|stable",
        .max_length = WM_CP_OTA_UPDATE_CHANNEL_LEN,
    },

#endif

    // LOGGING

    // ..
};

const uint16_t n_custom_params =
    sizeof(custom_params) / sizeof(t_cps_parameter);
