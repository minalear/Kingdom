#include "event.h"
#include "spdlog/spdlog.h"

void EventDispatcher::Subscribe(const EventFn &callback) {
  observers.push_back(callback);
}
void EventDispatcher::Post(const Event &event) const {
  // log only application level events
  if ((event.signature & EVENT_APPLICATION) == EVENT_APPLICATION) {
    spdlog::info("Event: {}", event.ToString());
  }

  for (const auto& observer : observers) {
    observer(event);
  }
}

/// Global event handler
EventDispatcher gEventHandler;