#pragma once

#include "Events.h"

class EventGPIO : public Event
{
  private:
    uint8_t pin;
    uint8_t level;

  public:
    EventGPIO(int delay, uint8_t pin, bool level) : Event(delay)
    {
        this->pin = pin;
        this->level = level;
        pinMode(pin, OUTPUT);
    }

    virtual void trigger()
    {
        digitalWrite(pin, level);
    }
};
