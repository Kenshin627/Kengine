#pragma once
#include "typedef.h"

class Window;

class Application
{
public:
	Application(uint width, uint height, const char* title);
	~Application();
	void Run();
	void initScene();
private:
	Scope<Window> mWindow;
};