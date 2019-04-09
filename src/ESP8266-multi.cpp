#include <Arduino.h>

#include "save_mode.h"
#include "http_server.h"
#include "config_load.h"
#include "common_vars.h"

void loopDelay(unsigned int delay)
{
  unsigned long start = millis();
  while (millis() <= start + delay)
  {
    WiFiConn.checkConnection(); // reconnect to wifi if connection lost

    ArduinoOTA.handle();

    if (mqtt_client) // MQTT LOOP
    {
      mqtt_client->loop();
    }

    event_manager.loop(); // EVENT MANAGER LOOOP

    serverHandleClient(); // HANDLE FILE SERVER

    udpHandle(); // handle light control by udp

    yield(); // dont crash if loop is taking long
  }
}

/*SETUP*/
void setup()
{
  DEBUG_LED.on();

  Serial.begin(115200);
  Serial.print("//garbage");
  Serial.print("\n\n---Setup START ---\n\n");

  bootSaveMode();

  WiFiConn.connect();

  bool config_loaded = loadConfig();
  if (!config_loaded)
  {
    setupOTA(NULL, NULL, false); // setup ota without mdns
  }

  serverInit();

  DEBUG_LED.off();
  Serial.print("\n---Setup END ---\n\n");
}

/*LOOP*/
void loop()
{
  // update sensors and lights only every 10 milliseconds
  loopDelay(10);

  { // UPDATE all SENSORS
    for (ISensor *sensor : sensors)
    {
      sensor->update();
    }
  }

  { // UPDATE all LED Strips
    bool power = false;
    for (LED_Strip *str : lights)
    {
      str->update();
      power |= str->getPower();
    }

    // trigger corresponding events depending on power state
    if (power)
    {
      event_manager.abortEvent("all_lights_off");
      event_manager.triggerEvent("any_light_on");
    }
    else
    {
      event_manager.abortEvent("any_light_on");
      event_manager.triggerEvent("all_lights_off");
    }
  }
}
