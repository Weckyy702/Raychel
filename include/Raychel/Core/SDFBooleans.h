/**
* \file SDFBooleans.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for boolean operations on SDFs
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
#ifndef RAYCHEL_SDF_BOOLEANS_H
#define RAYCHEL_SDF_BOOLEANS_H

#include "Types.h"

#include <cmath>

namespace Raychel {

    template <typename Target1, typename Target2>
    struct Union
    {
        Target1 target1;
        Target2 target2;
    };

    template <typename T1, typename T2>
    Union(T1, T2) -> Union<T1, T2>;

    template <typename T1, typename T2>
    double evaluate_sdf(const Union<T1, T2>& object, const vec3& p) noexcept
    {
        return std::min(evaluate_sdf(object.target1, p), evaluate_sdf(object.target2, p));
    }

    template <typename Target1, typename Target2>
    struct Difference
    {
        Target1 target1;
        Target2 target2;
    };

    template <typename T1, typename T2>
    Difference(T1, T2) -> Difference<T1, T2>;

    template <typename T1, typename T2>
    double evaluate_sdf(const Difference<T1, T2>& object, const vec3& p) noexcept
    {
        return std::max(-evaluate_sdf(object.target1, p), evaluate_sdf(object.target2, p));
    }

    template <typename Target1, typename Target2>
    struct Intersection
    {
        Target1 target1;
        Target2 target2;
    };

    template <typename T1, typename T2>
    Intersection(T1, T2) -> Intersection<T1, T2>;

    template <typename T1, typename T2>
    double evaluate_sdf(const Intersection<T1, T2>& object, const vec3& p) noexcept
    {
        return std::max(evaluate_sdf(object.target1, p), evaluate_sdf(object.target2, p));
    }

} // namespace Raychel

#endif //!RAYCHEL_SDF_BOOLEANS_H
