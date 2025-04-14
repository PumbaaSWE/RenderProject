#pragma once
#ifndef stuff
#define stuff

//#include <vulkan/vulkan.h>
#include <vector>
#include <iostream>
#include <string>
#include <optional>
#include <set>
#include <algorithm>





#define VK_USE_PLATFORM_WIN32_KHR //if windows


#include <vulkan/vulkan.h>

#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif


template<class ...Args>
void printl(Args&&... args) {
	(std::cout << ... << args) << '\n';
}

namespace tde {
	enum TdeResult { Success, Fail };
}

//REMOVE! this is so that stuff deas not get grayed out during development!!
//#define TDE_IMPLEMENTATION



#endif