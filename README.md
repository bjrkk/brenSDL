# brenSDL
An example (and POC) project showcasing the usage of BRender v1.1.2 with the SDL2 framework. This also works with the fixed-point and floating-point versions of BRender. The project is currently pretty messy, but feel free to look around.

This come prepackaged with BRender v1.1.2 libraries built using [my fork of BRender v1.1.2 with CMake support](https://www.libsdl.org/release/SDL2-devel-2.0.22-VC.zip). Don't bother with the old ones, as they don't work with this.

In order to build, you must have CMake, MSVC and the SDL2 VC development libraries. The contents of the SDL2 VC development libraries must be extracted into SDL2, specifically the contents of the folder bundled in the root of the archive. (e.g.: `SDL2-2.0.22`)

![brenSDL](img/screenshot.png)