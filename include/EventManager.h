#pragma once

#include <map>
#include <string>
#include "Events.h"
#include "EventGPIO.h"
#include <SerialStream.h>

class EventManager
{
  private:
    typedef std::map<std::string, std::vector<Event *>> MapEvents;

    MapEvents m_all_events;

  public:
    EventManager() {}

    inline void attachEvent(std::string on, Event *event)
    {
        m_all_events[on].push_back(event); // add event to vector. if onaction has not been set before it will be created.
    }

    inline void clearEvents(std::string on){
        m_all_events[on].clear();// clear all events from this onaction
    }

    void triggerEvent(std::string on)
    {
        MapEvents::iterator it = m_all_events.find(on);
        if (it != m_all_events.end()) // only if onaction was already found in eventlist
        {
            for (Event *event : it->second) // loop through all events stored for this onaction
            {
                event->schedule(); // reschedule event
            }
        }
    }

    void abortEvent(std::string on){
        MapEvents::iterator it = m_all_events.find(on);
        if (it != m_all_events.end()) // only if onaction was already found in eventlist
        {
            for (Event *event : it->second) // loop through all events stored for this onaction
            {
                event->abort(); // abort event
            }
        }
    }

    void loop()
    {
        for (const auto &vec : m_all_events)
        {
            for (Event *event : vec.second)
            {
                event->update();
            }
        }
    }
};
