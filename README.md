# Raychel - Raymarching Engine written in standard C++20

## What is ray marching?

In ray marching, every object is defined by a so called **signed distance funtion**, SDF for short. A SDF gives the distance between a point in space and a surface, with negative values meaning the point is inside the object.
To trace a ray in a certain direction, we can take the minimum of all SDFs in the scene and move by that amount in that direction. We can now repeat this process until the smallest distance is zero (or very close to it).
Like all ray tracing algorithms, ray marching allows for very realistic lighting simulations.

## Features

The engine is currently a WIP and new features are added with almost every commit.
Current features include:
- Basic Shapes
    - Spheres
    - Boxes
    - Planes
- SDF Modifiers
    - Hollowing
    - Rounded corners
    - Onioning
- SDF Transforms
    - Translation
    - Rotation
- SDF Boolean operations
    - Union
    - Difference
    - Intersection
- PBR Materials
    - Diffuse
    - Reflective (no glossy yet)
    - Refractive
- Denoising
    - Single scale implementation of the Ray Histogram Fusion algorithm
    - Multiscale (the good one) coming soon™
## Language

Raychel is written in standard C++20 using as little hacks as possible. The design style is rather functional than object oriented, although OO patterns are used.

## Building

### Compiler Support
Raychel is primarily developed on a GNU/Linux system, so changes that break Windows compatibility may not be fixed immediately. The following compilers work for sure:
* Clang 14
* GCC 12.2

### Dependencies / Library credit

No dependency management is currently implemented, so you have to install the following libraries manually:
* [RaychelLogger](https://github.com/Weckyy702/RaychelLogger)
* [RaychelCore](https://github.com/Weckyy702/RaychelCore)
* [RaychelMath](https://github.com/Weckyy702/RaychelMath)
* [GSL](https://github.com/microsoft/GSL)
* [Catch2](https://github.com/catchorg/Catch2) *only needed when building unit tests*

### How to build

After cloning the repository with

    git clone https://github.com/Weckyy702/Raychel

create the build directory using

    mkdir build; cd build

and run cmake:

    cmake <your options> ..

Lastly, build the library itself with

    make

<!--- If any errors occur, please check the [Wiki](https://github.com/Weckyy702/Raychel) TODO: add the wiki-->
## I found a bug!!

Nicely done! Please report it in the issues tab or open a pull request with your fix.
