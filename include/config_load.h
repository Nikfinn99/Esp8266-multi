
#pragma once

#include <Arduino.h>
#include <FS.h>
#include <ArduinoJson.h>
#include <WiFiConn.h>
#include <Upload-OTA.h>
#include <SerialStream.h>
#include <WiFiManager.h>
#include "common_vars.h"
#include "udp_lights.h"

void parseConfig(JsonObject &json);
void addLights(JsonArray &lights);
void addOtherDevices(JsonArray &device);
void loadEvents(JsonArray &events);
void loadMqtt(JsonObject &mqtt);
void loadOta(JsonObject &ota);

bool loadConfig(const char *path = nullptr)
{
    SPIFFS.begin();

    File configFile;
    if (!path)
    {
        configFile = SPIFFS.open("/config.json", "r");
    }
    else
    {
        configFile = SPIFFS.open(path, "r");
    }

    if (!configFile)
    {
        Serial.println("Failed to open config file");
        return false;
    }

    size_t size = configFile.size();
    if (size > 5000)
    {
        Serial.println("Config file size is too large");
        return false;
    }

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    configFile.readBytes(buf.get(), size);

    DynamicJsonBuffer jsonBuffer(1024);
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success())
    {
        Serial.println("Failed to parse config file");
        Serial.println("continuing to try to gain wifi connection");
        return false;
    }

    parseConfig(json);

    return true;
}

void parseConfig(JsonObject &json)
{
    // setup OTA
    loadOta(json["ota"].as<JsonObject>());

    // setup MQTT
    loadMqtt(json["mqtt"].as<JsonObject>());

    // setup LIGHTS
    addLights(json["lights"].as<JsonArray>());

    // setup DEVICES
    addOtherDevices(json["other"].as<JsonArray>());

    // setup EVENTS
    loadEvents(json["events"].as<JsonArray>());
}

void loadOta(JsonObject &ota)
{
    const char *name = ota["name"];
    const char *pw = ota["pw"];
    if (name && pw)
    {
        server_name = name;
        setupOTA(pw, name);
    }
    else
    {
        setupOTA();
        Serial << "OTA params not found!\n"
               << "Setting up OTA without params\n";
    }
    Serial << "set up OTA" << endl;
}

void loadMqtt(JsonObject &mqtt)
{
    const char *topic = mqtt["topic"];
    const char *server = mqtt["server"];
    const char *user = mqtt["user"];
    const char *pw = mqtt["pw"];
    if (topic && server && user && pw)
    {
        mqtt_client = new MQTT_Client(server, user, pw);
        mqtt_client->enableDebug(1);
        mqtt_client->setTopic(topic);
        mqtt_client->setCallback(mqtt_callback);

        mqtt_client->reconnect();
        Serial << "set up MQTT" << endl;
    }
}

void loadEvents(JsonArray &events)
{
    Serial << "setting up " << events.size() << " events: ";
    for (uint8_t i = 0; i < events.size(); i++)
    {
        JsonObject &event = events[i];
        const char *on = event["on"];
        int delay = event["delay"];

        JsonObject &evt = event["event"];
        const char *type = evt["type"];
        if (strlen(type) != 0 && strlen(on) != 0 && strcmp(type, "gpout") == 0) // if valid type and onaction and type == gpout
        {
            int pin = evt["pin"];
            int level = evt["level"];
            event_manager.attachEvent(on, new EventGPIO(delay, pin, level));
        }
        Serial << on << comma;
    }
    Serial << endl;
}

void addOtherDevices(JsonArray &devices)
{
    Serial << "setting up " << devices.size() << " other devices: ";
    for (uint8_t i = 0; i < devices.size(); i++)
    {
        JsonObject &device = devices[i];
        const char *type = device["type"];
        JsonArray &pins = device["pins"];

        if (strcmp(type, "ir_send") == 0) // IR SEND
        {
            int pin = pins[0];
            if (pin)
            {
                ir_send = new IRsend(pin); // new IR Send object enabling output
                ir_send->begin();
            }
        }
        Serial << type << comma;
    }
    Serial << endl;
}

void addLights(JsonArray &json_lights)
{
    Serial << "setting up " << json_lights.size() << " lights: ";
    for (uint8_t i = 0; i < json_lights.size(); i++)
    {
        JsonObject &light = json_lights[i];
        LED_Strip *str = nullptr;

        const char *name = light["name"];
        const char *type = light["type"];
        JsonArray &pins = light["pins"];

        if (strcmp(type, "pwm") == 0) // ANALOG WRITE PWM
        {
            const char *type_pwm = light["type_pwm"];
            if (strcmp(type_pwm, "rgb") == 0 && pins.size() == 3) // ANALOG RGB
            {
                str = new PWM_LED_Strip(pins[0], pins[1], pins[2]); // new pwm led strip
            }
        }
        else if (strcmp(type, "neopixel") == 0) // FASTLED WS2812
        {
            int pin = pins[0];
            int num_leds = light["num_leds"];
            switch (pin)
            {
            case 4:
                str = new FASTLED_Strip<WS2812, 4, GRB>(num_leds); // GPIO 4
                break;
            case 5:
                str = new FASTLED_Strip<WS2812, 5, GRB>(num_leds); // GPIO 5
                break;
            case 12:
                str = new FASTLED_Strip<WS2812, 12, GRB>(num_leds); // GPIO 12
                break;
            case 13:
                str = new FASTLED_Strip<WS2812, 13, GRB>(num_leds); // GPIO 13
                break;
            case 14:
                str = new FASTLED_Strip<WS2812, 14, GRB>(num_leds); // GPIO 14
                break;
            case 15:
                str = new FASTLED_Strip<WS2812, 15, GRB>(num_leds); // GPIO 15
                break;
            default:
                Serial << pin << " is not a Valid pin for NEOPIXEL\n";
            }

            // UDP Hyperion enable
            JsonObject &udp = light["udp"];
            if (udp.success())
            {
                Serial << "Loading UDP\n";
                int udp_num_leds = udp["num_leds"];
                int udp_timeout = udp["timeout"];
                Serial << "Udp num leds:" << udp_num_leds << "\n";
                udpSetStrip((Adressable_LED_Strip *)str, udp_num_leds);
                udpInit(udp_timeout);
            }
        }

        if (str) // only if valid strip was created
        {
            int tran_time = light["tran_time"];
            str->init(0, 0, tran_time); // init strip to use user transition time

            JsonArray &color_correction = light["color_correction"]; // get color correction from json
            if (color_correction.size() == 3)                        // check if valid
            {
                CRGB corr(color_correction[0], color_correction[1], color_correction[2]); // create CRGB object from color correction
                str->setColorCorrection(corr);
            }

            lights.push_back(str);       // append to vectors containing all lights
            light_names.push_back(name); // corresponding names
        }
        Serial << name << ":" << type << comma;
    }
    Serial << endl;
}