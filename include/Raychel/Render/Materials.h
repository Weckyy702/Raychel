/**
* \file Materials.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Materials class
* \date 2022-05-20
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
#ifndef RAYCHEL_MATERIALS_H
#define RAYCHEL_MATERIALS_H

#include "Raychel/Core/Types.h"

#include <cmath>

namespace Raychel {

    struct RenderState;

    struct ShadingData
    {
        vec3 position;
        vec3 normal;
        vec3 incoming_direction;

        const RenderState& state;
        std::size_t recursion_depth;
    };

    template <typename T>
    struct is_transparent_material : std::false_type
    {};

    template <typename T>
    constexpr bool is_transparent_material_v = is_transparent_material<T>::value;

    struct DeserializationErrorMaterial
    {};

    constexpr color get_surface_color(DeserializationErrorMaterial /*unused*/, const ShadingData& /*unused*/) noexcept
    {
        return color{1, 0, 1};
    }

} // namespace Raychel

#endif //!RAYCHEL_MATERIALS_H
