#pragma once
#ifndef GAME_ENGINE
#define GAME_ENGINE

#include <thread>
#include <atomic>
#include <iostream>
#include <chrono>
#include <map>
#include <functional>
#include <string>
#include <cassert>

#include "renderer.h"

//mybe use this?
#define PRINT_TO_CONSOLE

#ifdef _DEBUG
#define PRINT(x) std::cout << x << std::endl;
#else
#define PRINT(x) (void)0
#endif

//Time and Input
namespace tde {

	//https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
	inline std::wstring convert(const std::string& as);

	struct Time {
		static float deltaTime;
		static float fixedDeltaTime;
		static float time;
	};



	uint32_t constexpr numKeys = 256;
	enum KeyCode {
		NONE,
		A,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,
		Mouse0, //left
		Mouse1,	//right... should be middle?
		Mouse2,	//middle
		Mouse3,	//back
		Mouse4,	//fwd
	};

	struct Input //can I make this struct thing static? make two, one internal... or just have the rest be static/global
	{

		static bool newKeyState[numKeys];
		static bool oldKeyState[numKeys];
		static int mouseX;
		static int mouseY;
		static int mousePrevX;
		static int mousePrevY;
		static int mouseDeltaX;
		static int mouseDeltaY;


		static bool GetKey(KeyCode keyCode) {
			return newKeyState[keyCode];
		}

		static bool GetKeyDown(KeyCode keyCode) {
			return newKeyState[keyCode] && !oldKeyState[keyCode];
		}

		static bool GetKeyUp(KeyCode keyCode) {
			return !newKeyState[keyCode] && oldKeyState[keyCode];
		}

		//ALL INTERNALS COULD BE MOVE TO AN INTERNAL_INPUT STRUCT OR JUST BE GLOBAL STATIC FUNCS
		//Internal dont call
		static void UpdateKeyState(int keyCode, bool state) {
			newKeyState[keyCode] = state;
		}

		//Internal dont call
		static void UpdateKeys() {
			for (uint32_t i = 0; i < numKeys; i++)
			{
				oldKeyState[i] = newKeyState[i];
			}
			mouseDeltaX = 0;
			mouseDeltaY = 0;
			mousePrevX = mouseX;
			mousePrevY = mouseY;
		}

		//Internal dont call
		static void UpdateMousePos(int xPos, int yPos) {

			mouseX = xPos;
			mouseY = yPos;

			mouseDeltaX = mousePrevX - mouseX;
			mouseDeltaY = mousePrevY - mouseY;

		}

	};


}

//Application and Platforn decl
namespace tde {

	class Application; //HELLLOOOO!!!! I FORWAED DECLARED IT AS I SHOULD STUPID LINKER CAN YOU FIND IT ITS IN THE SAME FILE EVEN DONT BE STUPID PLEASE

	class Platform {
	public:
		virtual ~Platform() = default;
		virtual TdeResult CreateWindowPane(int width, int height, bool fullScreen) = 0;
		virtual TdeResult StartSystemEventLoop() = 0;
		virtual TdeResult ProcessSystemEvents() = 0;
		//virtual Renderer* CreateRenderer(int width, int height) = 0;
		virtual TdeResult SetWindowTitle(const std::string& s) = 0;
		static Application* app;
	};






	//static std::unique_ptr<Renderer> renderer;

	class Application {

		int width = 1920;
		int height = 1080;
		bool fullScreen = false;
		bool drawFrame = false;
		bool running = false;
		std::unique_ptr<Platform> platform;
		std::unique_ptr<Renderer> renderer;

		//std::function<void(int, int)> callback;

	public:
		std::string appName = "App Name Default";

		Application();
		~Application();

		TdeResult Create(int width, int height, bool fullScreen = false);

		TdeResult Start();

		auto getCurrentTime() {
			return std::chrono::high_resolution_clock::now();
		}


		void GameLoop();

		virtual void Init() = 0;
		virtual void FixedUpdate(float deltaTime);//make override frivillig?
		virtual void Update(float deltaTime) = 0;
		virtual void Render(float deltaTime, float extrapolation) = 0;
		virtual void Cleanup();

		void ProcessInput();

		//INTERNAL FUNCTIONS THAT ARE CALLED BY INTERNAL THINGS
		//this should be called "internally"... not by user (I think olc prefixes these with olc_ to differentiate)
		void OnSizeChanged(int newWidth, int newHeight);
		void OnClose();

		//END OF INTERNAL FUNCTIONS

		int GetScreenWidth();
		int GetScreenHeight();
		void ConfigurePlatform(); //end of file because the platforms are not defined otherwise (no .h header files...)
		Renderer& GetRenderer();
	};



}

/*
*	WINDOWS SPECIFICS
*
*/
#ifdef _WIN32
#ifndef UNICODE
#define UNICODE
#endif 

#include <windows.h>
#include <windowsx.h> // FOR SOME MACROS-.-
namespace tde {

	static std::map<size_t, uint8_t> keyMap; //nont like this being static but meh... we only run at one platform at a time
	class WindowsPlatform : public Platform {


		HWND hwnd = nullptr;

	public:

		virtual TdeResult CreateWindowPane(int width, int height, bool fullScreen) override;

		//callbacks from windows event queue, we deal with them here as they come in
		static LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		//this need to be in its own thread as it blocks, ProcessSystemEvents maybe could use peek message and be non blocking?
		TdeResult StartSystemEventLoop() override;

		//non blocking unless we have an endless amount of messages maybe...
		TdeResult ProcessSystemEvents() override;

		TdeResult SetWindowTitle(const std::string& s) override;

	};
}


#endif // !_WIN32
// END OF WINDOWS



//here are implementations!

#ifdef TOP_DOG_IMPLEMENTATION

//Time & Input impl + Platform static app default assignment
namespace tde {

	//we must initialize them outside because C++
	float Time::deltaTime = 0;
	float Time::fixedDeltaTime = 0.02f; //default 
	float Time::time = 0;


	//we must initialize them outside because C++
	bool Input::newKeyState[numKeys];
	bool Input::oldKeyState[numKeys];
	int Input::mouseX = 0;
	int Input::mouseY = 0;
	int Input::mousePrevX = 0;
	int Input::mousePrevY = 0;
	int Input::mouseDeltaX = 0;
	int Input::mouseDeltaY = 0;

	Application* Platform::app = nullptr;
}

//Application impl
namespace tde {

	Application::Application() {
		ConfigurePlatform();
	}
	Application::~Application() {}

	TdeResult Application::Create(int width, int height, bool fullScreen) {
		this->width = width;
		this->height = height;
		this->fullScreen = fullScreen;
		return Success;
	}

	TdeResult Application::Start() {


		renderer = std::make_unique<Renderer>();


		if (platform->CreateWindowPane(width, height, fullScreen) != Success) {
			return Fail;
		}

		running = true;

		GameLoop();



		return Success;
	}

	//https://gameprogrammingpatterns.com/game-loop.html
	void Application::GameLoop() {

		auto start = getCurrentTime();
		auto previous = getCurrentTime();
		float lag = 0.0;

		Init();

		int frameCount = 0;
		float frameTimer = 0;

		while (running)
		{
			auto current = getCurrentTime();
			
			std::chrono::duration<float> total = current - start;
			Time::time = total.count();

			std::chrono::duration<float> elapsed = current - previous;
			float deltaTime = elapsed.count();

			previous = current;
			lag += deltaTime;

			ProcessInput();

			while (lag >= Time::fixedDeltaTime)
			{
				Time::deltaTime = Time::fixedDeltaTime;
				FixedUpdate(Time::fixedDeltaTime);


				lag -= Time::fixedDeltaTime;
			}
			Time::deltaTime = deltaTime;
			Update(deltaTime);

			renderer->BeginFrame();

			Render(deltaTime, lag / Time::fixedDeltaTime);

			//renderer->Present();

			// update title bar and compute FPS - swiped from olcPixelGameEngine...
			frameTimer += deltaTime;
			frameCount++;
			if (frameTimer >= 1.0f)
			{
				//nLastFPS = frameCount;
				frameTimer -= 1.0f;
				std::string sTitle = "Top Dog Engine - " + appName + " - FPS: " + std::to_string(frameCount);
				platform->SetWindowTitle(sTitle);
				frameCount = 0;
			}


		}
		//renderer->WaitIdle();
		Cleanup();
	}

	void Application::FixedUpdate(float deltaTime) {}//make override frivillig?
	void Application::Cleanup() {}//make override frivillig?

	void Application::ProcessInput() {
		Input::UpdateKeys();
		platform->ProcessSystemEvents();
	};

	void Application::OnSizeChanged(int newWidth, int newHeight) {
		width = newWidth;
		height = newHeight;
		//renderer-> we can call the renderer here with the new size to recreate the image
		//PRINT("OnSizeChanged - width: " << newWidth << " height: " << newHeight);
	}

	void Application::OnClose() {
		PRINT("Closing window");
		running = false;
	}
	Renderer& Application::GetRenderer() {
		return *renderer;
	}

	int Application::GetScreenWidth() { return width; }
	int Application::GetScreenHeight() { return height; }




	void Application::ConfigurePlatform() {

		//https://stackoverflow.com/questions/4605842/how-to-identify-platform-compiler-from-preprocessor-macros
#ifdef _WIN32 // note the underscore: without it, it's not msdn official!
		// Windows (x64 and x86)
		platform = std::make_unique<WindowsPlatform>();
#elif __linux__
	// linux
#elif __APPLE__
	// Mac OS
#elif __unix__ // all unices, not all compilers
	// Unix
#endif

		//point back to app for callbacks
		platform->app = this; //what return value???????????
	}

}

#ifdef _WIN32
//Win32Platform
namespace tde {

	TdeResult WindowsPlatform::CreateWindowPane(int width, int height, bool fullScreen) {



		//set up the key map
		keyMap = {};
		keyMap[0x00] = KeyCode::NONE;
		keyMap[0x41] = KeyCode::A; keyMap[0x42] = KeyCode::B; keyMap[0x43] = KeyCode::C; keyMap[0x44] = KeyCode::D; keyMap[0x45] = KeyCode::E;
		keyMap[0x46] = KeyCode::F; keyMap[0x47] = KeyCode::G; keyMap[0x48] = KeyCode::H; keyMap[0x49] = KeyCode::I; keyMap[0x4A] = KeyCode::J;
		keyMap[0x4B] = KeyCode::K; keyMap[0x4C] = KeyCode::L; keyMap[0x4D] = KeyCode::M; keyMap[0x4E] = KeyCode::N; keyMap[0x4F] = KeyCode::O;
		keyMap[0x50] = KeyCode::P; keyMap[0x51] = KeyCode::Q; keyMap[0x52] = KeyCode::R; keyMap[0x53] = KeyCode::S; keyMap[0x54] = KeyCode::T;
		keyMap[0x55] = KeyCode::U; keyMap[0x56] = KeyCode::V; keyMap[0x57] = KeyCode::W; keyMap[0x58] = KeyCode::X; keyMap[0x59] = KeyCode::Y;
		keyMap[0x5A] = KeyCode::Z;



		// Register the window class.
		const wchar_t CLASS_NAME[] = L"Sample Window Class";

		WNDCLASS wc = { };

		wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
		wc.lpfnWndProc = WindowProc;
		wc.cbClsExtra = NULL;
		wc.cbWndExtra = NULL;
		wc.hInstance = GetModuleHandle(nullptr);
		wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		wc.hCursor = LoadCursor(NULL, IDC_ARROW);
		wc.hbrBackground = NULL;
		wc.lpszMenuName = NULL;
		wc.lpszClassName = CLASS_NAME;

		RegisterClass(&wc);


		int screenWidth = GetSystemMetrics(SM_CXSCREEN);
		int screenHeight = GetSystemMetrics(SM_CYSCREEN);

		//Not really correct
		if (fullScreen) {
			if ((width != (uint32_t)screenWidth) && (height != (uint32_t)screenHeight))
			{
				DEVMODE dmScreenSettings;
				memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
				dmScreenSettings.dmSize = sizeof(dmScreenSettings);
				dmScreenSettings.dmPelsWidth = width;
				dmScreenSettings.dmPelsHeight = height;
				dmScreenSettings.dmBitsPerPel = 32;
				dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
				if (ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
				{
					if (MessageBox(NULL, L"Fullscreen Mode not supported!\n Switch to window mode?", L"Error", MB_YESNO | MB_ICONEXCLAMATION) == IDYES)
					{
						fullScreen = false;
					}
					else
					{
						return Fail;
					}
				}
				screenWidth = width;
				screenHeight = height;
			}
		}


		DWORD dwExStyle;
		DWORD dwStyle;

		if (fullScreen)
		{
			dwExStyle = WS_EX_APPWINDOW;
			dwStyle = WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}
		else
		{
			dwExStyle = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
			dwStyle = WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;
		}

		RECT windowRect = {
			0L,
			0L,
			fullScreen ? (long)screenWidth : (long)width,
			fullScreen ? (long)screenHeight : (long)height
		};

		AdjustWindowRectEx(&windowRect, dwStyle, FALSE, dwExStyle);


		// Create the window.
		hwnd = CreateWindowEx(
			dwExStyle,                              // Optional window styles.
			CLASS_NAME,                     // Window class
			L"Top Dog Engine",    // Window text
			dwStyle,            // Window style

			// Size and position
			CW_USEDEFAULT, CW_USEDEFAULT,
			windowRect.right - windowRect.left,
			windowRect.bottom - windowRect.top,

			NULL,       // Parent window    
			NULL,       // Menu
			wc.hInstance,  // Instance handle
			NULL        // Additional application data
		);


		if (hwnd == NULL)
		{
			std::cout << "CreateWindowPane error code: " << GetLastError() << std::endl;
			return Fail;
		}

		//renderererereererere->do the thing
		//app->GetRenderer().Init({ hwnd, wc.hInstance }, width, height);

		app->GetRenderer().Create({ hwnd, wc.hInstance }, width, height);


		ShowWindow(hwnd, SW_SHOW);
		//SetForegroundWindow(hwnd);
		//SetFocus(hwnd);
		return Success;
	}

	//callbacks from windows event queue, we deal with them here as they come in
	LRESULT CALLBACK WindowsPlatform::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {

		assert(app);

		switch (uMsg)
		{
		case WM_SIZE:
		{
			int width = LOWORD(lParam);  // Macro to get the low-order word.
			int height = HIWORD(lParam); // Macro to get the high-order word.
			app->OnSizeChanged(width, height);
			return 0;
		}
		case WM_KEYDOWN:	Input::UpdateKeyState(keyMap[wParam], true);                      return 0;
		case WM_KEYUP:		Input::UpdateKeyState(keyMap[wParam], false);                     return 0;

		case WM_LBUTTONDOWN:Input::UpdateKeyState(Mouse0, true);                              return 0;
		case WM_LBUTTONUP:	Input::UpdateKeyState(Mouse0, false);                             return 0;
		case WM_RBUTTONDOWN:Input::UpdateKeyState(Mouse1, true);                              return 0;
		case WM_RBUTTONUP:	Input::UpdateKeyState(Mouse1, false);                             return 0;
		case WM_MBUTTONDOWN:Input::UpdateKeyState(Mouse2, true);                              return 0;
		case WM_MBUTTONUP:	Input::UpdateKeyState(Mouse2, false);                             return 0;
		case WM_XBUTTONDOWN:Input::UpdateKeyState(Mouse2 + HIWORD(wParam), true);             return 0;
		case WM_XBUTTONUP:	Input::UpdateKeyState(Mouse2 + HIWORD(wParam), false);            return 0;

		case WM_MOUSEMOVE:
		{
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			Input::UpdateMousePos(xPos, yPos);
			return 0;
		}



		case WM_CLOSE:		app->OnClose();                                                   return 0;
		case WM_DESTROY:
			PostQuitMessage(0);
			DestroyWindow(hWnd);
			return 0;
		default:
			return DefWindowProc(hWnd, uMsg, wParam, lParam);
		}
		return 0;
		//return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}

	//this need to be in its own thread as it blocks, ProcessSystemEvents maybe could use peek message and be non blocking?
	TdeResult WindowsPlatform::StartSystemEventLoop() {
		return Success;
	}

	//non blocking unless we have an endless amount of messages maybe...
	TdeResult WindowsPlatform::ProcessSystemEvents() {
		MSG msg = { };
		while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		return Success;
	}

	TdeResult WindowsPlatform::SetWindowTitle(const std::string& s)
	{
#ifdef UNICODE
		size_t len = s.size() + 1;
		wchar_t* buf = new wchar_t[len];
		swprintf(buf, len, L"%S", s.c_str());
		SetWindowText(hwnd, buf);
		delete[] buf;
#else
		SetWindowText(hwnd, s.c_str());
#endif
		return Success;
	}

}
#endif //_WIN32

//static helper funcs
namespace tde {
	//https://stackoverflow.com/questions/6691555/converting-narrow-string-to-wide-string
	inline std::wstring convert(const std::string& as)
	{
		size_t len = as.size() + 1;
		wchar_t* buf = new wchar_t[len]; //*2+2???
		swprintf(buf, len, L"%S", as.c_str());
		std::wstring rval = buf;
		delete[] buf;
		return rval;
	}
}
#endif // !TOP_DOG_IMPLEMENTATION



#endif // !GAME_ENGINE