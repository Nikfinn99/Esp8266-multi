
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

bool parseConfig(JsonObject &json);
bool addLight(JsonObject &light, int tran_time);
bool addOtherDevice(JsonObject &device);

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

    StaticJsonBuffer<2000> jsonBuffer;
    JsonObject &json = jsonBuffer.parseObject(buf.get());

    if (!json.success())
    {
        Serial.println("Failed to parse config file");
        Serial.println("continuing to try to gain wifi connection");
        return false;
    }

    return parseConfig(json);
}

bool parseConfig(JsonObject &json)
{
    bool success = true;

    { //WIFI
        const char *ssid = json["wifi"]["ssid"];
        const char *pw = json["wifi"]["pw"];
        bool wifi_connected = WiFiConn.connect(ssid, pw);
        success &= wifi_connected;

        if (!wifi_station_connect())
        {
            Serial << "WIFI connection unsuccesfull!"
                   << "\n";
            Serial << "Opening WiFiManager Portal"
                   << "\n";
            WiFiManager wm;
            wm.startConfigPortal();
            ESP.restart();
            while (1)
            {
                delay(100);
            }
        }
    } //wifi

    { //OTA
        const char *name = json["ota"]["name"];
        const char *pw = json["ota"]["pw"];
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
    } //ota

    { //MQTT
        JsonObject &mqtt = json["mqtt"];
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

            success &= mqtt_client->reconnect();
        }
    } //mqtt

    { //LIGHTS
        int tran_time = json["lights"]["tran_time"];
        JsonArray &devices = json["lights"]["devices"];

        for (uint8_t i = 0; i < devices.size(); i++)
        {
            success &= addLight(devices[i], tran_time);
        }

    } //lights

    { // OTHER DEVICE
        JsonArray &devices = json["other"];
        for (uint8_t i = 0; i < devices.size(); i++)
        {
            success &= addOtherDevice(devices[i]);
        }
    } // other device

    { // EVENTS
        JsonArray &events = json["events"];

        for (uint8_t i = 0; i < events.size(); i++)
        {
            JsonObject &event = events[i];
            const char *on = event["on"];
            int delay = event["delay"];

            JsonObject &evt = event["event"];
            const char *type = evt["type"];
            if (type && on && strcmp(type, "gpout") == 0) // if valid type and onaction and type == gpout
            {
                int pin = evt["pin"];
                int level = evt["level"];
                event_manager.attachEvent(on, new EventGPIO(delay, pin, level));
            }
        }
    } // events

    return success;
}

bool addOtherDevice(JsonObject &device)
{
    const char *type = device["type"];
    JsonArray &pins = device["pins"];

    if (strcmp(type, "ir_send") == 0) // IR SEND
    {
        int pin = pins[0];
        if (pin)
        {
            ir_send = new IRsend(pin); // new IR Send object enabling output
            ir_send->begin();
            return true;
        }
    }
    return false;
}

bool addLight(JsonObject &light, int tran_time)
{
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
        else
        {
            return false; // not a valid output option for pwm
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
    else
    {
        return false; // not a valid light type
    }

    if (str) // only if valid strip was created
    {
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

    return true;
}