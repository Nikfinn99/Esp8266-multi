#pragma once

#include <WiFiUdp.h>
#include <LED_Str.h>
#include <SerialStream.h>

WiFiUDP _udp;
Adressable_LED_Strip *_udp_strip;
byte *_udpBuffer;
uint16_t _bufferSize;
uint16_t _ledCount;
uint16_t _udpPort = 19446;
bool _udp_enabled = false;
unsigned long _udp_timeout = 0;
int _udp_max_timeout = 0;

void udpInit(int timeout) // create new udp object and init with port
{
    _udp = WiFiUDP();
    _udp.begin(_udpPort);
    _udp_enabled = true;
    _udp_max_timeout = timeout;
}

void udpSetStrip(Adressable_LED_Strip *strip, int num_lights)
{
    _udp_strip = strip;

    _ledCount = num_lights;      // get number of lights from input instead of strip
    _bufferSize = _ledCount * 3; //3 bytes per LED

    _udpBuffer = new byte[_bufferSize + 1];
    _udpBuffer[_bufferSize] = 0;
}

void udpHandle() // parse packet and write to leds
{
    if (_udp_enabled)
    {
        int bytes = _udp.parsePacket();
        if (bytes > 0)
        {
            if (bytes >= _bufferSize)
            {
                Serial << "UDP Receiving packet:" << bytes << "\n";
                uint32_t total_brightness = 0;
                _udp.readBytes(_udpBuffer, _bufferSize);
                for (int i = 0; i < _ledCount; i++)
                {
                    uint8_t r = _udpBuffer[i * 3 + 0];
                    uint8_t g = _udpBuffer[i * 3 + 1];
                    uint8_t b = _udpBuffer[i * 3 + 2];
                    total_brightness += r + g + b;

                    CRGB color = CRGB(r, g, b);
                    if (_udp_strip)
                    {
                        _udp_strip->setSingleColor(color, i);
                    }
                }
                if (total_brightness != 0)
                {
                    _udp_timeout = millis();
                }
            }
            else
            {
                Serial << "UDP-Packet size expected=" << _bufferSize << ", actual=" << bytes << "\n";
            }
        }
    }

    if (_udp_timeout + _udp_max_timeout < millis())
    {
        _udp_strip->setMode(LED_Strip::MODE_SINGLE);
    }
}