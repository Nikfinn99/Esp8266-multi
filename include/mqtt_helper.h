#ifndef MQTT_HELPER_H_
#define MQTT_HELPER_H_

#include <ArduinoJson.h>
#include <string>
#include <IRsend.h>
#include <FastLED.h>

#include <LED_Str.h>

#include "define.h"
#include "common_vars.h"

void mqtt_dev_ir(const std::string &action, const std::string &payload);
void mqtt_dev_light(const std::string &action, const std::string &payload, LED_Strip *light);

void mqtt_callback(const std::string &device, const std::string &action, const std::string &payload)
{
    event_manager.triggerEvent("mqtt_callback");

    for (uint8_t i = 0; i < light_names.size(); i++)
    {
        if (device == light_names[i])
        {
            mqtt_dev_light(action, payload, lights[i]);
            // no break to allow multiple lights with the same name
        }
    }

    if (device == "ir")
    { //IR Send
        mqtt_dev_ir(action, payload);
    }

    if (mqtt_client) //publish to confirm change
    {
        mqtt_client->publish(device + "/" + action, payload);
        event_manager.triggerEvent("mqtt_stat_published");
    }
}

void mqtt_dev_ir(const std::string &action, const std::string &payload)
{
    if (payload.length() == 0) //abort if there is no payload
        return;

    event_manager.triggerEvent("mqtt_ir_callback");

    if (action == "send" && ir_send)
    {
        DynamicJsonBuffer buffer;
        JsonObject &root = buffer.parseObject(payload.c_str());

        if (root.success())
        {
            const char *type = root["type"];
            const char *comm_str = root["comm"];
            int bits = root["bits"];

            if (type && comm_str)
            {
                uint64_t comm = strtoul(comm_str, nullptr, 0);

                if (strcmp(type, "NEC") == 0)
                {
                    ir_send->sendNEC(comm, bits);
                    event_manager.triggerEvent("mqtt_ir_sent");
                }
                else if (strcmp(type, "SONY") == 0)
                {
                    ir_send->sendSony(comm, bits);
                    event_manager.triggerEvent("mqtt_ir_sent");
                }
            }
        }
    }
}

void mqtt_dev_light(const std::string &action, const std::string &payload, LED_Strip *light)
{
    if (!light)
        return; // abort if light is nullptr

    if (payload.length() == 0) //abort if there is no payload
        return;

    event_manager.triggerEvent("mqtt_light_callback");

    if (action == "rgb") //rgb
    {
        size_t firstCommaIndex = payload.find(',');
        size_t secondCommaIndex = payload.find(',', firstCommaIndex + 1);
        if (firstCommaIndex == std::string::npos || secondCommaIndex == std::string::npos)
        {
            Serial.println("Invalid RGB String!");
        }
        else //both indexes were found
        {
            std::string red = payload.substr(0, firstCommaIndex - 0);
            std::string green = payload.substr(firstCommaIndex + 1, secondCommaIndex - (firstCommaIndex + 1));
            std::string blue = payload.substr(secondCommaIndex + 1);

            CRGB color;
            color.r = atoi(red.c_str());
            color.g = atoi(green.c_str());
            color.b = atoi(blue.c_str());

            light->setColor(color);

            event_manager.triggerEvent("light_rgb_set");
        }
    }                           //rgb
    else if (action == "power") //power
    {
        if (payload == "ON")
        {
            light->setPower(1);
            event_manager.triggerEvent("light_turned_on");
        }
        else
        {
            light->setPower(0);
            event_manager.triggerEvent("light_turned_off");
        }
    }                         //power
    else if (action == "bri") //brightness
    {
        light->setBrightness(atoi(payload.c_str()));
        event_manager.triggerEvent("light_bri_set");
    } //brightness
}

#endif //MQTT_HELPER_H_