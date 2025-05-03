#pragma once
#ifndef stuff
#define stuff

#include <vector>
#include <iostream>
#include <fstream>
#include <string>
#include <optional>
#include <set>
#include <algorithm>
#include <functional>
#include <deque>
#include <cassert>
#include <span>
#include <array>



#define VK_USE_PLATFORM_WIN32_KHR //if windows
#define DISABLE_VULKAN_OBS_CAPTURE = 1

#include <vulkan/vulkan.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifdef _DEBUG
#define DEBUGPRINTL(x)																											\
{																																\
	std::cout << "line: " << __LINE__ << " file: " << __FILE__ << " func: " << __func__ << "msg: " << x << std::endl;			\
}																																\																										
#else
#define DEBUGPRINTL(x) ((void)0);
#endif


#define VK_CHECK(x)                                                     \
    do {                                                                \
        VkResult err = x;                                               \
        if (err) {                                                      \
             std::cout << "line: " << __LINE__ << " file: " << __FILE__ << " func: " << __func__ << "msg: " << err << std::endl;\
            abort();                                                    \
        }                                                               \
    } while (0)

#define USING_GLM

#ifdef USING_GLM
#include <glm\glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

using mat4_t = glm::mat4;
using vec3_t = glm::vec3;
using vec2_t = glm::vec2;
#endif // USING_GLM

template<class ...Args>
void printl(Args&&... args) {
	(std::cout << ... << args) << '\n';
}

namespace tde {
	enum TdeResult { Success, Fail };

}


struct DeletionQueue
{
	std::deque<std::function<void()>> deletors;

	void push_function(std::function<void()>&& function) {
		deletors.push_back(function);
	}

	void flush() {
		// reverse iterate the deletion queue to execute all the functions
		for (auto it = deletors.rbegin(); it != deletors.rend(); it++) {
			(*it)(); //call functors
		}

		deletors.clear();
	}
};


//REMOVE! this is so that stuff deas not get grayed out during development!!
//#define TDE_IMPLEMENTATION



#endif