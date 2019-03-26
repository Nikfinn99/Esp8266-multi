#ifndef INCLUDE_H
#define INCLUDE_H

/*default libs*/
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <FS.h>
#define FASTLED_ESP8266_RAW_PIN_ORDER
#define FASTLED_ALLOW_INTERRUPTS 0
#include <FastLED.h>
#include <ArduinoJson.h>
#include <IRsend.h>
#include <IRutils.h>
#include <vector>
#include <DHT.h>

/*Wifi Manager*/
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h> //https://github.com/tzapu/WiFiManager

/*nikfinn99*/
#include <LED_Str.h>
#include <Upload-OTA.h>
#include <Filter.h>
#include <IO.h>
#include "MQTT_Client.h"
#include <SerialStream.h>
#include "mqtt_helper.h"
#include <WiFiConn.h>
#include "http_server.h"
#include "config_load.h"
#include "common_vars.h"

#include "define.h"

#endif // INCLUDE_H
