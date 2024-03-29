/**
* \file RenderHelper.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for RenderHelper class
* \date 2022-04-12
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
#ifndef RAYCHEL_RENDER_UTILS_H
#define RAYCHEL_RENDER_UTILS_H

#include "Renderer.h"

namespace Raychel {

    struct RefractionData
    {
        vec3 surface_point;
        vec3 incoming_direction;
        vec3 normal;

        double material_ior;
        double ior_variation;

        const RenderState& state;
        std::size_t recursion_depth;
    };

    [[nodiscard]] color get_shaded_color(const RenderData& data) noexcept;

    [[nodiscard]] color get_diffuse_lighting(const ShadingData& data) noexcept;

    [[nodiscard]] color get_refraction(const RefractionData& data) noexcept;
} // namespace Raychel

#endif //!RAYCHEL_RENDER_UTILS_H
