# Esp8266-multi

## Installation
The Project uses Git Submodules.
Therefore you must clone the repository recursively.
Otherwise the lib/* folders will be empty and compilation will not work.

`git clone --recursive https://github.com/Nikfinn99/Esp8266-multi.git`

As an alternative you could use GitHub-Desktop

## Compilation
The Project uses PlatformIO.
If you have not installed it yet, follow instructions on https://platformio.org/

1. Open the Project in PlatformIO and the necessary dependencies should be downloaded.
2. Compile and upload project to esp8266
3. Create your own `config.json` from `config_example.json`
<br> **This config Example is not a valid config as the pins are used multiple times**
3. Upload config as `config.json` to the ESP SPIFFS root:
- (option 1) Upload your config file to the esp using instructions at 
<br> https://docs.platformio.org/en/latest/platforms/espressif8266.html#uploading-files-to-file-system-spiffs
- (option 2) Upload the SPIFFS Image from the Arduino FSBrowser example and open `http://[ESP-IP]/edit` and upload your config there 
<br> https://github.com/esp8266/Arduino/tree/master/libraries/ESP8266WebServer/examples/FSBrowser
- (option 3) Upload my modified version of FSBrowser data from the *releases* using my ESP-Ota-UI Tool open the same URL as above

5. Reboot ESP to load config
6. Configure your Home Automation software to send correct packets over MQTT 
<br> (use retained messages for lights as the esp does not store the last power state)
- MQTT commands for lights follow the scheme:
<br> cmnd/[mqtt.topic]/[lights[i].name]/rgb -> RED,GREEN,BLUE in range from 0 to 255
<br> cmnd/[mqtt.topic]/[lights[i].name]/bri -> BRIGHTNESS in range from 0 to 255
<br> cmnd/[mqtt.topic]/[lights[i].name]/power -> POWER as either ON or OFF
7. For lights to turn on RGB, Brightness and Power State has to be initialized once
