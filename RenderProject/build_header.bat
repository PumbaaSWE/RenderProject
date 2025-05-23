@ECHO OFF

REM Clear file but writing header info to it

type HeaderInfoText.h > single_header\tdeTopDogEngine.h

REM This abomination is supposedly a new line... 
Echo( >> single_header\tdeTopDogEngine.h


REM dont use *.h because the order of copy is important!

call:append stuff.h
call:append init_helper.h
call:append vk_init.h
call:append vk_images.h
call:append obj_loader.h
call:append Pipeline.h
call:append Swapchain.h
call:append renderer.h
call:append Model.h
call:append application.h

Echo( >> single_header\tdeTopDogEngine.h
Echo #ifdef TOP_DOG_IMPLEMENTATION >> single_header\tdeTopDogEngine.h

REM Not all .h-file have corresponding .cpp files
REM would be nice to strip all #includes ".*" from this?

call:append init_helper.cpp
call:append vk_init.cpp
call:append vk_images.cpp
call:append Pipeline.cpp
call:append Swapchain.cpp
call:append renderer.cpp
call:append Model.cpp
call:append application.cpp


Echo( >> single_header\tdeTopDogEngine.h
Echo #endif //TOP_DOG_IMPLEMENTATION >> single_header\tdeTopDogEngine.h

echo Did it!

pause
exit

REM append all lines from a file unless it begins with #include " or reallt NOT < ... but eh
:append
REM type %~f1 >> single_header\tdeTopDogEngine.h
findstr /V /R /C:"#include [^<]" %~f1 >> single_header\tdeTopDogEngine.h
Echo( >> single_header\tdeTopDogEngine.h
