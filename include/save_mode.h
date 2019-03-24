#pragma once

#include <Arduino.h>
#include <user_config.h>
#include <SerialStream.h>
#include <WiFiConn.h>
#include <ArduinoOTA.h>
#include <Upload-OTA.h>

void bootSaveMode()
{
    rst_info *reset_info;
    delay(200); // slow down so we really can see the reason!!
    reset_info = ESP.getResetInfoPtr();
    if (reset_info->reason == rst_reason::REASON_EXCEPTION_RST) // esp crashed! safe mode
    {
        Serial << "ESP CRASHED!"
               << "\n"
               << "BOOTING IN SAVE MODE"
               << "\n";
        WiFiConn.checkConnection();
        setupOTA();

        while (1)
        {
            ArduinoOTA.handle();
            delay(200);
            DEBUG_LED.toggle();
        }
    }
}