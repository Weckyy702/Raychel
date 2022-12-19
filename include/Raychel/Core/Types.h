/**
* \file Types.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Types class
* \date 2022-04-10
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
#ifndef RAYCHEL_TYPES_H
#define RAYCHEL_TYPES_H

#include "RaychelMath/Transform.h"
#include "RaychelMath/color.h"
#include "RaychelMath/vec2.h"
#include "RaychelMath/vec3.h"

#include <functional>

namespace Raychel {

    using vec3 = basic_vec3<double>;
    using color = basic_color<double>;
    using Quaternion = basic_quaternion<double>;
    using Transform = basic_transform<double>;

    using Size2D = basic_vec2<std::size_t>;

    class SDFContainer;

    class Scene;

    struct RenderData;

    using BackgroundFunction = std::function<color(const RenderData&)>;

    template <typename T>
    struct DeserializationTag
    {};

    template <typename T>
    struct has_target : std::false_type
    {};

    template <typename T>
    constexpr bool has_target_v = has_target<T>::value && requires(T t){
        t.target;
    };

} // namespace Raychel

#endif //!RAYCHEL_TYPES_H
