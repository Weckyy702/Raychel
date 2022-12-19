/**
* \file SDFModifiers.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for modifiers on SDFs
* \date 2022-05-27
*
* MIT License
* Copyright (c) [2022] [Weckyy702 (weckyy702@gmail.com | https://github.com/Weckyy702)]
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
*/
#ifndef RAYCHEL_SDF_MODIFIERS_H
#define RAYCHEL_SDF_MODIFIERS_H

#include "Raychel/Core/Types.h"

namespace Raychel {

    template <typename Target>
    struct Hollow
    {
        Target target;
    };

    template <typename T>
    Hollow(T) -> Hollow<T>;

    template <typename T>
    double evaluate_sdf(const Hollow<T>& object, const vec3& p) noexcept
    {
        return std::abs(evaluate_sdf(object.target, p));
    }

    template <typename Target>
    struct Rounded
    {
        Target target;
        double radius;
    };

    template <typename T>
    Rounded(T, double) -> Rounded<T>;

    template <typename T>
    double evaluate_sdf(const Rounded<T>& object, const vec3& p) noexcept
    {
        return evaluate_sdf(object.target, p) - object.radius;
    }

    template <typename Target>
    struct Onion
    {
        Target target;
        double thickness;
    };

    template <typename T>
    Onion(T, double) -> Onion<T>;

    template <typename T>
    double evaluate_sdf(const Onion<T>& object, const vec3& p) noexcept
    {
        return std::abs(evaluate_sdf(object.target, p)) - object.thickness;
    }

} // namespace Raychel

#endif //!RAYCHEL_SDF_MODIFIERS_H
