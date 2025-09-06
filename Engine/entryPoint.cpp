#include "application.h"
#include "logger.h"
#include "core/graphic/program/shader.h"
int main()
{
	Log::Init();
	Application app(1920, 1080, "Kenshin Engine");
	KS_CORE_INFO("Kenshin Engine Initialized!");
	app.Run();
	return 0;	
}