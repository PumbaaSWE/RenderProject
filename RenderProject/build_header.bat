REM This abomination is supposedly a new line...
Echo( >> single_header\tdeTopDogEngine.h


REM dont use *.h because the order of copy is important!

type stuff.h >> single_header\tdeTopDogEngine.h
type init_helper.h vk_init.h >> single_header\tdeTopDogEngine.h
type obj_loader.h >> single_header\tdeTopDogEngine.h
type Pipeline.h >> single_header\tdeTopDogEngine.h
type Swapchain.h >> single_header\tdeTopDogEngine.h
type renderer.h >> single_header\tdeTopDogEngine.h
type Model.h >> single_header\tdeTopDogEngine.h
type application.h >> single_header\tdeTopDogEngine.h

Echo( >> single_header\tdeTopDogEngine.h
Echo #ifdef TDE_IMPLEMENTATION >> single_header\tdeTopDogEngine.h

REM Not all .h-file have corresponding .cpp files
REM would be nice to strip all #includes from this?

type init_helper.cpp vk_init.cpp >> single_header\tdeTopDogEngine.h
type Pipeline.cpp >> single_header\tdeTopDogEngine.h
type Swapchain.cpp >> single_header\tdeTopDogEngine.h
type renderer.cpp >> single_header\tdeTopDogEngine.h
type Model.cpp >> single_header\tdeTopDogEngine.h
type application.cpp >> single_header\tdeTopDogEngine.h


Echo( >> single_header\tdeTopDogEngine.h
Echo #endif //TDE_IMPLEMENTATION  >> single_header\tdeTopDogEngine.h
pause