#ifndef __IIZI_WEBSERVER_H__
#define __IIZI_WEBSERVER_H__

#include "ESPAsyncWebServer.h"

AsyncWebServer *webserver_init();
AsyncWebServer *webserver_instance();

#endif  // __IIZI_WEBSERVER_H__
