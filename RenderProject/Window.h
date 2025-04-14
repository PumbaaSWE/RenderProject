#pragma once
#include "stuff.h"
class Window
{
protected:
	std::string name;
	int width;
	int height;
	bool fullscreen;
public:

	//virtual Window(std::string name, int width, int height, bool fullscreen = false) = 0;
	virtual ~Window() = default;
	static Window& Create(const std::string& name, int width, int height, bool fullScreen);
	//virtual TdeResult StartSystemEventLoop() = 0;
	virtual bool ProcessSystemEvents() = 0;
	//virtual Renderer* CreateRenderer(int width, int height) = 0;
	virtual bool SetWindowTitle(const std::string& s) = 0;
};

#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h> // FOR SOME MACROS-.-
class Win32Window : public Window {
	HWND hwnd = nullptr;
	HINSTANCE hinstance = nullptr;
	static Win32Window* self;
public:
	Win32Window(const std::string& name, int width, int height, bool fullScreen);
	~Win32Window() override;
	virtual bool ProcessSystemEvents() override;
	virtual bool SetWindowTitle(const std::string& s) override;
};
#endif //_WIN32

#ifdef TDE_IMPLEMENTATION



Window& Window::Create(const std::string& name, int width, int height, bool fullScreen) {
	//https://stackoverflow.com/questions/4605842/how-to-identify-platform-compiler-from-preprocessor-macros
#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
		// Windows (x64 and x86)
	Win32Window window(name, width, height, fullScreen);
	return window;
#elif __linux__
	// linux
#elif __APPLE__
	// Mac OS
#elif __unix__ // all unices, not all compilers
	// Unix
#endif
}

#ifdef _WIN32

Win32Window* Win32Window::self = nullptr;

Win32Window::Win32Window(const std::string& name, int width, int height, bool fullScreen)
{
	if (self != nullptr) {
		throw ("We only support one window at a time!");
	}
	this->name = name;
	this->width = width;
	this->height = height;
	this->fullscreen = fullScreen;
	self = this;
}
Win32Window::~Win32Window(){

}

bool Win32Window::ProcessSystemEvents() {
	return true;
}

bool Win32Window::SetWindowTitle(const std::string& s) {
	return true;
}
#endif //_WIN32



#endif //TDE_IMPLEMENTATION

