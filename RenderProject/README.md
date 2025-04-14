Hello

To eventually, as required by specs, move all into a sigle header-file
I have prepared an ordering
stuff.h is on top, it includes the basics and non og the written files
after that init_helper
then renderer in cludes that
application includes renderer
finaly main includes application 

Larger files are split into .h and .cpp for readability (I dont like C++s two file system, just do like C# or smth...)

Smaller files have declarations on top and then a guard around the implementation
to prevent errors with duplicated definitions

that means in order to get the definition you need to #define THAT_IMPLEMENTATION
all theses guards should be the fame in the monolithic file at the end, but now they are 
unique to their file to not accidentaly do harm. Se application.cpp for example

Window class is experimental and should not be used.

stuff:
important definitions, includes, and tiny helper funcs

init_helper:
Helper construction methods and classes (WIP)
TODO: better builders with data flowing better... like vkBootloader?

renderer:
Vulkan rendering code (WIP)
TODO: next add swapchain + ...the rest

application:
Window creation, Input handling and GameLoop.

main:
main + example usage and testing

-Jack