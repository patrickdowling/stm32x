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
- Update to c++14/17 (\*)
- Formalize GPIO (and possibly peripheral) export into the base project.
- Externalize the directories for specific families to separate repos?
- Support for different models within the families is sketchy at best (\*\*)
- Better support for other debugger/upload options than the Black Magic Probe; openocd was used in some projects but hasn't made it to main makefile yet.

(\*)
- Possible using the `STM32X_CPPSTD` variable. The default C++ version is c++11.
- Difficutly: this may require newer versions of CMSIS is required since the `register` keyword is deprecated.
- As a somewhat temporary solution CMSIS is now being used directly and _not_ the ST packaged version.
- C++17 seems to work, C++20 will be more interesting given `volatile` is deprecated.

(\*\*)
- In general it seems useful to use the ST `STM32CubeF0` et al. repositories...
- ...the but there is that the model #defines have changed so mixing newer ST with StdPeriphLib seems dangerous.

## Stuff
- Obviously inspired by [stmlib](https://github.com/pichenettes/stmlib)!
