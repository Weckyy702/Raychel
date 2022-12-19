/**
* \file Raymarch.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for Raymarch class
* \date 2022-06-11
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

#include "Raychel/Core/Raymarch.h"
#include "Raychel/Core/SDFContainer.h"

namespace Raychel {

    std::pair<double, std::size_t> evaluate_distance_field(const std::vector<SDFContainer>& surfaces, const vec3& point) noexcept
    {
        const auto surfaces_size = surfaces.size();

        double min_distance{1e9};
        auto hit_index = no_hit;
        for (std::size_t i{}; i != surfaces_size; ++i) {
            const auto surface_distance = std::abs(surfaces[i].evaluate(point));

            if (surface_distance < min_distance) {
                hit_index = i;
                min_distance = surface_distance;
            }
        }
        return {min_distance, hit_index};
    }

    RaymarchResult raymarch(
        vec3 current_point, const vec3& direction, const std::vector<SDFContainer>& surfaces, RaymarchOptions options) noexcept
    {
        double depth{};
        std::size_t step{};
        while (step != options.max_ray_steps && depth < options.max_ray_depth) {
            const auto [max_distance, hit_index] = evaluate_distance_field(surfaces, current_point);
            if (max_distance < options.surface_epsilon) {
                return {current_point, depth, step, hit_index};
            }
            current_point += direction * max_distance;
            depth += max_distance;
            ++step;
        }
        return {current_point, depth, step, no_hit};
    }

    vec3 get_normal(const vec3& point, const SDFContainer& surface, double normal_offset) noexcept
    {
        //This implements the tetrahedon sampling technique found at https://iquilezles.org/articles/normalsSDF/
        constexpr vec3 xyy{1, -1, -1}, yyx{-1, -1, 1}, yxy{-1, 1, -1}, xxx{1, 1, 1};

        if (surface.has_custom_normal()) {
            return surface.get_normal(point);
        }

        // clang-format off
        return normalize(vec3{
            xyy*surface.evaluate(point+xyy*normal_offset) +
            yyx*surface.evaluate(point+yyx*normal_offset) +
            yxy*surface.evaluate(point+yxy*normal_offset) +
            xxx*surface.evaluate(point+xxx*normal_offset)
       });
        // clang-format on
    }

} // namespace Raychel
