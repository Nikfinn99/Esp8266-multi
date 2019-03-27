#include <Arduino.h>
#include "include.h"
#include <user_interface.h> // https://github.com/esp8266/Arduino actually tools/sdk/include
#include "save_mode.h"

void loopDelay(unsigned int delay)
{
  unsigned long start = millis();
  while (millis() <= start + delay)
  {
    WiFiConn.checkConnection();

    ArduinoOTA.handle();

    if (mqtt_client) // MQTT LOOP
    {
      mqtt_client->loop();
    }

    event_manager.loop(); // EVENT MANAGER LOOOP

    serverHandleClient(); // HANDLE FILE SERVER

    udpHandle();

    yield();
  }
}

/*SETUP*/
void setup()
{
  DEBUG_LED.on();

  Serial.begin(115200UL, SERIAL_8N1, SERIAL_TX_ONLY, 1);
  Serial.print("//garbage");
  Serial.print("\n\n---Setup START ---\n\n");

  bootSaveMode();

  WiFiConn.connect();

  bool config_loaded = loadConfig();
  if (!config_loaded)
  {
    setupOTA(NULL, NULL, false);// setup ota without mdns
  }

  serverInit();

  DEBUG_LED.off();
  Serial.print("\n---Setup END ---\n\n");
}

/*LOOP*/
void loop()
{
  loopDelay(10);

  // UPDATE all LED Strips
  bool power = false;
  for (LED_Strip *str : lights)
  {
    str->update();
    power |= str->getPower();
  }

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

  // UPDATE all SENSORS
  for(ISensor* sensor : sensors)
  {
    sensor->update();
  }
  
}
