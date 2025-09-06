#include "event.h"

bool Event::isHandled() const
{
	return mHandled;
}

void Event::setHandled(bool handle)
{
	mHandled = handle;
}

void Event::makeHandled()
{
	mHandled = true;
}