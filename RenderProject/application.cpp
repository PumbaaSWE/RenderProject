#include "application.h"

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

			renderer->EndFrame();
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
		renderer->WaitIdle();
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

		keyMap[0x30] = KeyCode::Alpha0; keyMap[0x31] = KeyCode::Alpha1; keyMap[0x32] = KeyCode::Alpha2; keyMap[0x33] = KeyCode::Alpha3; keyMap[0x34] = KeyCode::Alpha4;
		keyMap[0x35] = KeyCode::Alpha5; keyMap[0x36] = KeyCode::Alpha6; keyMap[0x37] = KeyCode::Alpha7; keyMap[0x38] = KeyCode::Alpha8; keyMap[0x39] = KeyCode::Alpha9;


		keyMap[VK_UP] = KeyCode::UP; keyMap[VK_DOWN] = KeyCode::DOWN; keyMap[VK_LEFT] = KeyCode::LEFT; keyMap[VK_RIGHT] = KeyCode::RIGHT;

		keyMap[VK_SPACE] = KeyCode::SPACE; keyMap[VK_TAB] = KeyCode::TAB; keyMap[VK_SHIFT] = KeyCode::SHIFT; keyMap[VK_CONTROL] = KeyCode::CTRL;
		keyMap[VK_INSERT] = KeyCode::INS; keyMap[VK_DELETE] = KeyCode::DEL; keyMap[VK_HOME] = KeyCode::HOME;
		keyMap[VK_END] = KeyCode::END; keyMap[VK_PRIOR] = KeyCode::PGUP; keyMap[VK_NEXT] = KeyCode::PGDN;
		keyMap[VK_BACK] = KeyCode::BACK; keyMap[VK_ESCAPE] = KeyCode::ESCAPE; keyMap[VK_RETURN] = KeyCode::ENTER; keyMap[VK_PAUSE] = KeyCode::PAUSE; keyMap[VK_SCROLL] = KeyCode::SCROLL;


		//keyMap[VK_OEM_1] = KeyCode::OEM_1;			// On US and UK keyboards this is the ';:' key
		//keyMap[VK_OEM_2] = KeyCode::OEM_2;			// On US and UK keyboards this is the '/?' key
		//keyMap[VK_OEM_3] = KeyCode::OEM_3;			// On US keyboard this is the '~' key
		//keyMap[VK_OEM_4] = KeyCode::OEM_4;			// On US and UK keyboards this is the '[{' key
		//keyMap[VK_OEM_5] = KeyCode::OEM_5;			// On US keyboard this is '\|' key.
		//keyMap[VK_OEM_6] = KeyCode::OEM_6;			// On US and UK keyboards this is the ']}' key
		//keyMap[VK_OEM_7] = KeyCode::OEM_7;			// On US keyboard this is the single/double quote key. On UK, this is the single quote/@ symbol key
		//keyMap[VK_OEM_8] = KeyCode::OEM_8;			// miscellaneous characters. Varies by keyboard
		//keyMap[VK_OEM_PLUS] = KeyCode::EQUALS;		// the '+' key on any keyboard
		//keyMap[VK_OEM_COMMA] = KeyCode::COMMA;		// the comma key on any keyboard
		//keyMap[VK_OEM_MINUS] = KeyCode::MINUS;		// the minus key on any keyboard
		//keyMap[VK_OEM_PERIOD] = KeyCode::PERIOD;	// the period key on any keyboard
		//keyMap[VK_CAPITAL] = KeyCode::CAPS_LOCK;

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