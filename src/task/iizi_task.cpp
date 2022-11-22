
#include "../portal/iizi_portal.h"
#include "../storage/wifi_credential_storage.h"
#include "../wifi/iizi_wifi.h"
#include "WiFi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "is_timer_expired.h"

bool iizi_portal_should_open     = false;
bool iizi_portal_should_close    = false;
bool iizi_wifi_should_start_scan = false;

static void handle_portal_close_request() {
  // no creds stored yet? leave it open no matter what! =)
  if (wcs_stored_credentials_count() <= 0) return;

  iizi_portal_close();
}

static void check_portal_not_used() {
  static TickTimer close_ap_timer = TickTimer(60000);
  const auto now                  = millis();

  // portal closed? nothing to do here..
  if (!iizi_portal_is_open()) {
    close_ap_timer.reset(now);
    return;
  }

  // clients connected? reset timer and leave for now!
  if (iizi_portal_clients_connected() > 0) {
    close_ap_timer.reset(now);
    return;
  }

  // no creds stored yet?
  if (wcs_stored_credentials_count() <= 0) {
    close_ap_timer.reset(now);
    return;
  }

  // WiFi not connected? reset timer and leave..
  if (WiFi.status() != WL_CONNECTED) {
    close_ap_timer.reset(now);
    return;
  }

  // timer still ticking?
  if (close_ap_timer.isTicking(now)) return;
  close_ap_timer.reset(now);

  Serial.println("time to close the AP.. ;)");

  // AP open, no one connected, WiFi status seems fine
  // .. and our timer is expired?
  // => lets close the portal
  iizi_portal_should_close = true;
}

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

      iizi_wifi_start_a_scan();
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
