#ifndef KINGDOM_EVENT_H
#define KINGDOM_EVENT_H

#include <functional>
#include <vector>
#include "SDL2/SDL_scancode.h"

enum class EventType {
  ApplicationClose,
  MouseMove,
  KeyboardButtonUp,
  KeyboardButtonDown
};

// Used for event filtering
const uint8_t EVENT_APPLICATION = 0b00000010;
const uint8_t EVENT_OTHER       = 0b00000001;

struct Event {
  EventType type;
  uint8_t signature = 0x00;

  explicit Event(EventType type, uint8_t sig)
      : type(type), signature(sig) { }
  virtual const char* ToString() const = 0;
};
struct ApplicationCloseEvent : Event {
  ApplicationCloseEvent()
      : Event(EventType::ApplicationClose, EVENT_APPLICATION) { }

  const char* ToString() const override {
    return "ApplicationClose";
  }
};
struct MouseMoveEvent : Event {
  int x, y;

  MouseMoveEvent(int x, int y)
      : Event(EventType::MouseMove, EVENT_OTHER),
        x(x), y(y) { }

  const char* ToString() const override {
    return "MouseMove";
  }
};
struct KeyboardEvent : Event {
  SDL_Scancode key;

  KeyboardEvent(EventType eventType, SDL_Scancode key)
      : Event(eventType, EVENT_OTHER),
        key(key) { }

  const char* ToString() const override {
    return "KeyboardEvent";
  }
};

// auto const& mouse = dynamic_cast<const MouseMoveEvent&>(event);

typedef std::function<void(const Event&)> EventFn;

class EventDispatcher {
  std::vector<EventFn> observers;

public:
  void Subscribe(const EventFn& callback);
  void Post(const Event& event) const;
};

extern EventDispatcher gEventHandler;

#endif //KINGDOM_EVENT_H
