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
		Alpha0,
		Alpha1,
		Alpha2,
		Alpha3,
		Alpha4,
		Alpha5,
		Alpha6,
		Alpha7,
		Alpha8,
		Alpha9,
		UP, DOWN, LEFT, RIGHT,
		SPACE, TAB, SHIFT, CTRL, INS, DEL, HOME, END, PGUP, PGDN,
		BACK, ESCAPE, RETURN, ENTER, PAUSE, SCROLL,

		OEM_1, OEM_2, OEM_3, OEM_4, OEM_5, OEM_6, OEM_7, OEM_8,
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

		/*!
		Is the key pressed currently
		*/
		static bool GetKey(KeyCode keyCode) {
			return newKeyState[keyCode];
		}
		/*!
		Was the key pressed down this frame
		*/
		static bool GetKeyDown(KeyCode keyCode) {
			return newKeyState[keyCode] && !oldKeyState[keyCode];
		}
		/*!
		Was the key released down this frame
		*/
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

		//std::function<void(int, int)> callback;

	public:
		std::unique_ptr<Renderer> renderer;
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



#endif // !GAME_ENGINE