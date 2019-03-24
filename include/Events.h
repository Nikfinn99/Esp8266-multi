#pragma once

#include <Arduino.h>

class Event
{
  private:
    unsigned long start;
    int delay;
    bool active;

  public:
    Event(int delay) : start(0), delay(delay), active(false) {}

    inline void enable()
    {
        setEnabled(true);
    }

    inline void disable()
    {
        setEnabled(false);
    }

    inline void abort()
    {
        setEnabled(false);
    }

    inline void setEnabled(bool enabled)
    {
        active = enabled;
    }

    inline void schedule(int delay)
    {
        if (!active)
        {
            start = millis();
            delay = delay;
            setEnabled(true);
        }
    }

    inline void schedule()
    {
        if (!active)
        {
            start = millis();
            setEnabled(true);
        }
    }

    inline void reschedule()
    {
        start = millis();
        setEnabled(true);
    }

    inline bool isEnabled()
    {
        return active;
    }

    inline int getDelay()
    {
        return delay;
    }

    void update()
    {
        if (active)
        {
            if (millis() >= start + delay)
            {
                trigger();
                setEnabled(false);
            }
        }
    }

    virtual void trigger() = 0;
};
