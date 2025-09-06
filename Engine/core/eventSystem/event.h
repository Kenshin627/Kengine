#pragma once
#include <typeindex>

class Event
{
public:
	Event() = default;
	~Event() = default;
	Event(const Event& e) = delete;
	Event& operator=(const Event& e) = delete;
	bool isHandled() const;
	void setHandled(bool handle);
	void makeHandled();
	virtual std::type_index getType() const = 0;
	virtual const char* getName() const = 0;
private:
	bool mHandled{ false };
};