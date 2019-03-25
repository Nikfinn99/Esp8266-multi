#ifndef MQTT_HELPER_H_
#define MQTT_HELPER_H_

#include <ArduinoJson.h>
#include <string>
#include <IRsend.h>
#include <FastLED.h>

#include <LED_Str.h>

#include "define.h"
#include "common_vars.h"

void mqtt_dev_ir(const String &action, const String &payload);
void mqtt_dev_light(const String &action, const String &payload, LED_Strip *light);

void mqtt_callback(const String &device, const String &action, const String &payload)
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

void mqtt_dev_ir(const String &action, const String &payload)
{
    if (payload.length() == 0) //abort if there is no payload
        return;

    event_manager.triggerEvent("mqtt_ir_callback");

    if (action == "send" && ir_send)
    {
        DynamicJsonBuffer buffer;
        JsonObject &root = buffer.parseObject(payload);

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

void mqtt_dev_light(const String &action, const String &payload, LED_Strip *light)
{
    // abort if light is nullptr
    if (!light)
        return;

    // abort if there is no payload
    if (payload.length() == 0)
        return;

    event_manager.triggerEvent("mqtt_light_callback");

    if (action == "rgb") //rgb
    {
        int firstCommaIndex = payload.indexOf(',');
        int secondCommaIndex = payload.indexOf(',', firstCommaIndex + 1);

        if (firstCommaIndex < 0 || secondCommaIndex < 0)
        {
            Serial << "Invalid RGB String!" << endl;
        }
        else //both indexes were found
        {
            String red = payload.substring(0, firstCommaIndex);
            String green = payload.substring(firstCommaIndex + 1, secondCommaIndex);
            String blue = payload.substring(secondCommaIndex + 1);

            CRGB color;
            color.r = red.toInt();
            color.g = green.toInt();
            color.b = blue.toInt();

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
        light->setBrightness(payload.toInt());
        event_manager.triggerEvent("light_bri_set");
    } //brightness
}

#endif //MQTT_HELPER_H_