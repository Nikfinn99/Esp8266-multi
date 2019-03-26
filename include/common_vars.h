#pragma once

#include <MQTT_Client.h>
#include <EventManager.h>

//VARS
MQTT_Client *mqtt_client;
IRsend *ir_send;
std::vector<LED_Strip *> lights;
std::vector<String> light_names;
EventManager event_manager;
String server_name;

//FUNCTIONS
void mqtt_callback(const String &device, const String &action, const String &payload);