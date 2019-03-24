#pragma once

#include <MQTT_Client.h>
#include "EventManager.h"

//VARS
MQTT_Client *mqtt_client;
IRsend *ir_send;
std::vector<LED_Strip *> lights;
std::vector<std::string> light_names;
EventManager event_manager;
std::string server_name;

//FUNCTIONS
void mqtt_callback(const std::string &device, const std::string &action, const std::string &payload);