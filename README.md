# STM32 Build System
## What is it?
- Yet-another-stm32 build thing.
- Answer to the question "can I export the GPIO assignments from CubeMX to C++?". The answer is yes, with some (XML) parsing.
- Experimental at best.

## Why is it?
- NIH/learning by f\*ck up.

## How does it work?
- Basically, add `stm32x` as a subdirectory/-module in the project.
- Create a suitable makefile that sets the right things and `include stm32x/makefile` (e.g. [oCf4](https://github.com/patrickdowling/oCf4))
- The CubeMX export requires some addtitional steps and isn't part of of the makefile.

## Should I use it?
- Probably not. There are working prototypes of things running successfully on F0, F4 and F373 boards but it was mostly for my amusement and a vehicle to figure out how things fit together. I tend to do that by re-implementing things from the ground up.

## Quo Vadis
- More templates, fewer macros.
- Update to c++14/17
- Formalize GPIO (and possibly peripheral) export into the base project.
- Externalize the directories for specific families to separate repos?
- Better support for other debugger/upload options than the Black Magic Probe; openocd was used in some projects but hasn't made it to main makefile yet.

## Stuff
- Obviously inspired by [stmlib](https://github.com/pichenettes/stmlib)!
