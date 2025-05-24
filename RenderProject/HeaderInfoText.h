#pragma once
/*
* the big header
*
* To include it in multiple files create a tdeTopDogEngine.cpp file that only has the following:

	#define TOP_DOG_IMPLEMENTATION
	#include "tdeTopDogEngine.h"

* Nothing else!
* Then you can include tdeTopDogEngine.h anywhere without duplicating
* It also improves compile times as you do not recompile the whole engine each time...
*
* =====HOW TO SET UP!=====
*
* Make Sure to have Vulkan SDK installed (https://www.lunarg.com/vulkan-sdk/)
* When installing, make sure to include the "GLM Headers" component.
*
* Make sure the Enviroment variable VULKAN_SDK is set to the installation folder
* probably done when installing the SDK
* in cmd: echo %VULKAN_SDK%
* result should be something like: C:\VulkanSDK\1.3.211.0
* if not set it in cmd: setx VULKAN_SDK "C:\VulkanSDK\1.3.211.0"
* in PowerShell:
* [Environment]::SetEnvironmentVariable('VULKAN_SDK','C:\VulkanSDK\1.3.211.0')
* [Environment]::GetEnvironmentVariable('VULKAN_SDK')
* 
* Restart Visual Studio after properly setting up VULKAN_SDK
*
* Set up Visual Studio: (Right click Your Project under Solution in Solution Explorer>Properties)
*
* Properties>General>C++ Language Standard
* Select ISO C++20
*
* Properties>Linker>All Options>Additional Dependencies
* Add vulkan-1.lib
*
* Properties>Linker>All Options>Additional Library Directories
* Add %VULKAN_SDK%\Lib
*
* Properties>C/C++>All Options>Additional Include Directories
* Add %VULKAN_SDK%\Include
*
*/

//Example main.cpp
/*
#define TOP_DOG_IMPLEMENTATION
#include "tdeTopDogEngine.h"


class Application1 : public tde::Application
{

	float time = 0;
	int secs = 0;
	bool showTime = false;

	tde::Model plane;


public:
	Application1()
	{
		appName = "Top Dog";
	}

	void Init() override {
		plane = tde::Model(&GetRenderer(), tde::Model::cube_verts, tde::Model::cube_indices);
	}


	void FixedUpdate(float dt) override {

	}


	void Update(float dt) override {


	}
	void Render(float dt, float extrapolation) override {


		mat4_t proj = glm::perspective(glm::radians(60.0f), 720.0f / 420.0f, 0.1f, 1000.0f); //this only change when fov or zNear/zFar changes
		proj[1][1] *= -1; //glm is flipped (OpenGL v Vulkan up? y neg up or down?)

		glm::mat4 view = glm::mat4(1.0f);
		view = glm::translate(view, { 0, 0, -10 });


		glm::mat4 identity = glm::mat4(1.0f); //this is currently not used
		renderer->SetUniformBuffer(identity, view, proj);



		glm::mat4 model = glm::mat4(1.0f);
		model = glm::rotate(model, 60.0f, { 0,1,1 }); // rotate around the y axis
		plane.Draw(model);

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::translate(model2, { .6, 0, 8 });
		model2 = glm::rotate(model2, 3.0f, { 0,1,0 }); // rotate around the y axis
		plane.Draw(model2);

		glm::mat4 model3 = glm::mat4(1.0f);
		model3 = glm::translate(model3, { 0, .7, 4 });
		model3 = glm::rotate(model3, 70.0f, { 1,1,0 }); // rotate around the y axis
		plane.Draw(model3);
	}

};





int main()
{
	Application1 app;
	if (app.Create(720, 420) == tde::Success)
		app.Start();
	return 0;
}
*/

/*
	Authors
	Jack Forsberg
	Johannes Widén

	Copyright (c) 2025 Team Alpha Top Dog Ace Squad
	All rights reserved.

	*--------------------*
	|                    |
	|     A cool box     |
	|                    |
	*--------------------*
*/


/*
	Change-log
	
	Version 0.0
	2025-05-19
	- First ever test of the software as one header with Vulkan 1.3
	- Read obj files, limitations: needs to be triangulated, contain no submeshes, less than 16000 verts, possibly some more
	- One hardcoded pipeline
	- Two hardecoded default shaders (vert and frag) in binary form
	- Support to read shaders in SPRV format
	- One default cube to render
	- Windows support
	
	Version 0.1
	2025-05-20
	- Fixed bug with precompiled shaders were defined in the header
	- Added authors and copyright

	Version 0.2
	2025-05-23
	- Added more KeyCodes
	- Added Known bugs to this 

	Version 0.3
	2025-05-24
	- Updated set up instructions
	- Updated Known bugs
*/

/*
	KNOWN BUGS:
	- Minimizing is not handled properly and validation layers freak out about it
	- Changing resolution while running is not handled properly and can cause a crash in some cases
*/