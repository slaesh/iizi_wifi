
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "iizi_portal.h"
#include "iizi_wifi.h"
#include "storage/wifi_credential_storage.h"

bool iizi_portal_should_open     = false;
bool iizi_portal_should_close    = false;
bool iizi_wifi_should_start_scan = false;

static void handle_portal_close_request() {
  // no creds stored yet? leave it open no matter what! =)
  if (wcs_stored_credentials_count() <= 0) return;

  iizi_portal_close();
}

static void handle_wifi_scan_start() { iizi_wifi_start_a_scan(); }

static void iizi_task(void *_) {
  if (wcs_stored_credentials_count() == 0) {
    iizi_portal_open();
  }

  for (;;) {
    vTaskDelay(10 / portTICK_PERIOD_MS);

    iizi_wifi_loop();

    if (iizi_portal_should_close) {
      iizi_portal_should_close = false;

      handle_portal_close_request();
    }

    if (iizi_wifi_should_start_scan) {
      iizi_wifi_should_start_scan = false;

      handle_wifi_scan_start();
    }

    if (iizi_portal_should_open) {
      iizi_portal_should_open = false;

      // TODO: timeout?
      iizi_portal_open();
    }
  }
}

void iizi_create_task() {
  xTaskCreate(iizi_task, "iizi", 1024 * 4, NULL, 1, NULL);
}
