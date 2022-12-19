/**
* \file Raymarch.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Raymarch class
* \date 2022-04-11
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
#ifndef RAYCHEL_RAYMARCH_H
#define RAYCHEL_RAYMARCH_H

#include "Types.h"

#include <limits>
#include <utility>
#include <vector>

namespace Raychel {

    constexpr static auto no_hit = std::numeric_limits<std::size_t>::max();

    struct RaymarchResult
    {
        vec3 point{};
        double ray_depth{};
        std::size_t ray_steps{};
        std::size_t hit_index{no_hit};
    };

    struct RaymarchOptions
    {
        std::size_t max_ray_steps{1'000};
        double max_ray_depth{100};
        double surface_epsilon{1e-3};
    };

    [[nodiscard]] std::pair<double, std::size_t>
    evaluate_distance_field(const std::vector<SDFContainer>& surfaces, const vec3& point) noexcept;

    [[nodiscard]] RaymarchResult raymarch(
        vec3 current_point, const vec3& direction, const std::vector<SDFContainer>& surfaces, RaymarchOptions options) noexcept;

    [[nodiscard]] vec3 get_normal(const vec3& point, const SDFContainer& surface, double normal_offset = 1e-6) noexcept;

} // namespace Raychel

#endif //!RAYCHEL_RAYMARCH_H
