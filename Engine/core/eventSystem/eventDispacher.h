#pragma once
#include "event.h"
#include <functional>
#include <type_traits>
#include <typeindex>
#include <memory>
#include <unordered_map>
#include <vector>
#include "core.h"


class EventDispatcher
{
public:
	typedef uint64_t EventListenerId;
	using EventLisnerMap = std::unordered_map<std::type_index, std::vector<std::pair<EventListenerId, std::function<void(Event&)>>>>;
public:
	EventDispatcher() = default;
	virtual ~EventDispatcher() = default;
	EventDispatcher(const EventDispatcher& d) = delete;
	EventDispatcher& operator=(const EventDispatcher& d) = delete;
	template<typename EventType>
	EventListenerId addEventListener(std::function<void(EventType&)> listener)
	{
		KS_CORE_ASSERT(std::is_base_of<Event, EventType>::value, "EventType must be derived from Event");
		auto eventTypeId = typeid(EventType);
		auto wrapper = [listener](Event& e)
		{
			EventType& specificEvent = static_cast<EventType&>(e);
			listener(e);
		}
		EventListenerId listenerId = GeneraterId();
		mEventListeners[eventTypeId].emplace_back(listenerId, wrapper);
		return listenerId;
	}

	template<typename EventType>
	bool removeEventListener(EventListenerId listenerId)
	{
		KS_CORE_ASSERT(std::is_base_of<Event, EventType>::value, "EventType must be derived from Event");
		auto iter = mEventListeners.find(typeid(EventType));
		if (iter == mEventListeners.end())
		{
			return false;
		}
		auto& listeners = iter->second;
		for (auto& listener : listeners)
		{
			if (listener->first == listenerId)
			{
				listeners.erase(it);
				return true;
			}
		}
		return false;
	}

	void clearAll()
	{
		mEventListeners.clear();
	}

	template<typename EventType>
	bool removeEventListeners()
	{
		KS_CORE_ASSERT(std::is_base_of<Event, EventType>::value, "EventType must be derived from Event");
		auto iter = mEventListeners.find(typeid(EventType));
		if (iter != mEventListeners.end())
		{
			mEventListeners.erase(iter);
			return true;
		}
		return false;
	}

	template<typename EventType>
	void dispatchEvent(EventType& event)
	{
		KS_CORE_ASSERT(std::is_base_of<Event, EventType>::value, "EventType must be derived from Event");
		auto iter = mEventListeners.find(typeid(EventType));
		if (iter != mEventListeners.end())
		{
			auto listeners = iter->second;
			for (auto& listener : listeners)
			{
				listener->second(event);
				if (event.isHandled())
				{
					return;
				}
			}
		}
	}
private:
	EventListenerId GeneraterId() { return mNextGeneratorId++; }
private:	
	EventListenerId mNextGeneratorId{ 1 };
	EventLisnerMap mEventListeners;
};
