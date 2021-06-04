#ifndef KINGDOM_EVENT_H
#define KINGDOM_EVENT_H

#include <functional>
#include <vector>

enum class EventType {
  ApplicationClose
};

struct Event {
  EventType type;
  explicit Event(EventType type) : type(type) { }
  virtual const char* ToString() const = 0;
};
struct ApplicationCloseEvent : Event {
  ApplicationCloseEvent() : Event(EventType::ApplicationClose) { }
  const char* ToString() const override {
    return "ApplicationClose";
  }
};

typedef std::function<void(const Event&)> EventFn;

class EventDispatcher {
  std::vector<EventFn> observers;

public:
  void Subscribe(const EventFn& callback);
  void Post(const Event& event) const;
};

extern EventDispatcher gEventHandler;

#endif //KINGDOM_EVENT_H
