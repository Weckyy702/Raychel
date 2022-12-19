/**
* \file RenderUtils.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for RenderUtils class
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

#include "Raychel/Render/RenderUtils.h"
#include <iterator>
#include "Raychel/Core/Raymarch.h"
#include "Raychel/Core/ZigguratNormal.h"

#include "RaychelMath/vector.h"

namespace Raychel {

    color get_shaded_color(const RenderData& data) noexcept
    {
        const auto get_background_color = [&] {
            if (data.state.get_background) {
                return data.state.get_background(data);
            }
            return color{data.direction.x(), data.direction.y(), data.direction.z()};
        };

        if (data.recursion_depth >= data.state.options.max_recursion_depth) {
            return get_background_color();
        }

        const auto& [surfaces, materials, _, options] = data.state;
        const auto result = raymarch(
            data.origin, data.direction, surfaces, {options.max_ray_steps, options.max_ray_depth, options.surface_epsilon});

        if (result.hit_index == no_hit) {
            return get_background_color();
        }

        const auto surface_normal = get_normal(result.point, surfaces[result.hit_index], options.normal_epsilon);
        RAYCHEL_ASSERT(equivalent(mag_sq(surface_normal), 1.0));

        return data.state.materials[result.hit_index].get_surface_color(
            {.position = result.point + surface_normal * options.shading_epsilon,
             .normal = surface_normal,
             .incoming_direction = data.direction,
             .state = data.state,
             .recursion_depth = data.recursion_depth + 1U});
    }

    static vec3 get_random_direction_on_weighted_hemisphere(const vec3& normal) noexcept
    {
        vec3 test{};
        do {
            test = normal + vec3{ziggurat_normal(), ziggurat_normal(), ziggurat_normal()};
        } while (test == vec3{});

        test = normalize(test);

        if (dot(test, normal) < 0.0) {
            test *= -1;
        }
        return test;
    }

    color get_diffuse_lighting(const ShadingData& data) noexcept
    {
        const auto direction = get_random_direction_on_weighted_hemisphere(data.normal);
        return get_shaded_color(RenderData{
                   .origin = data.position,
                   .direction = direction,
                   .state = data.state,
                   .recursion_depth = std::max(
                       data.state.options.max_recursion_depth - data.state.options.max_lighting_bounces, data.recursion_depth)}) *
               dot(direction, data.normal);
    }

    [[nodiscard]] static double fresnel(const vec3& direction, vec3 normal, double interior_ior, double exterior_ior)
    {
        RAYCHEL_ASSERT(equivalent(mag_sq(direction), 1.0));
        RAYCHEL_ASSERT(equivalent(mag_sq(normal), 1.0));

        auto cosi = dot(direction, normal);

        if (cosi > 0.0) {
            std::swap(interior_ior, exterior_ior);
            normal *= -1.0;
        } else {
            cosi *= -1.0;
        }

        const auto etai = exterior_ior;
        const auto etat = interior_ior;

        const auto sint = (exterior_ior / interior_ior) * std::sqrt(std::max(0.0, 1.0 - sq(cosi)));

        if (sint >= 1.0) {
            return 1.0; //TIR
        }

        const auto cost = std::sqrt(1.0 - sq(sint));

        const auto Rs = ((etat * cosi) - (etai * cost)) / ((etat * cosi) + (etai * cost));
        const auto Rp = ((etai * cosi) - (etat * cost)) / ((etai * cosi) + (etat * cost));

        return std::clamp((sq(Rs) + sq(Rp)) * 0.5, 0.0, 1.0);
    }

    [[nodiscard]] static vec3 refract(const vec3& direction, vec3 normal, double interior_ior, double exterior_ior)
    {
        RAYCHEL_ASSERT(equivalent(mag_sq(direction), 1.0));
        RAYCHEL_ASSERT(equivalent(mag_sq(normal), 1.0));

        auto cosi = dot(direction, normal);

        if (cosi > 0.0) {
            std::swap(interior_ior, exterior_ior);
            normal *= -1.0;
        } else {
            cosi *= -1.0;
        }

        const auto eta = exterior_ior / interior_ior;

        const auto k = 1.0 - sq(eta) * (1.0 - sq(cosi));
        if (k < 0.0) {
            return vec3{}; //TIR
        }
        return normalize((direction * eta) + (normal * (eta * cosi - std::sqrt(k))));
    }

    static std::size_t get_surrounding_object(const std::vector<SDFContainer>& surfaces, const vec3& point) noexcept
    {
        std::size_t hit_index{no_hit};
        double min_distance{-1e9};
        for (std::size_t i{}; i != surfaces.size(); ++i) {
            const auto object_distance = surfaces[i].evaluate(point);
            if (object_distance < 0.0 && object_distance > min_distance) {
                min_distance = object_distance;
                hit_index = i;
            }
        }
        return hit_index;
    }

    static double get_surrounding_ior(
        const vec3& surface_point, const std::vector<SDFContainer>& surfaces, const std::vector<MaterialContainer>& materials)
    {
        const auto closest_object_index = get_surrounding_object(surfaces, surface_point);
        if (closest_object_index == no_hit) {
            return 1.0;
        }
        return materials[closest_object_index].get_material_ior();
    }

    [[nodiscard]] static color get_reflective_component(const RefractionData& data, double reflection_factor) noexcept
    {
        if (reflection_factor < 0.01) {
            return color{};
        }
        return get_shaded_color(
                   {.origin = data.surface_point,
                    .direction = reflect(data.incoming_direction, data.normal),
                    .state = data.state,
                    .recursion_depth = data.recursion_depth}) *
               reflection_factor;
    }

    [[nodiscard]] static color get_refractive_component(const RefractionData& data, double ior_factor, double outer_ior) noexcept
    {
        const auto trace_direction = refract(data.incoming_direction, data.normal, data.material_ior * ior_factor, outer_ior);
        const auto trace_origin = data.surface_point - ((2.0 * data.state.options.shading_epsilon) * data.normal);

        if (trace_direction == vec3{}) {
            Logger::warn("Did not expect to reach ", __FILE__, ':', __LINE__, '\n');
            return color{0};
        }

        const auto& options = data.state.options;
        const auto result = raymarch(
            trace_origin,
            trace_direction,
            data.state.surfaces,
            {options.max_ray_steps, options.max_ray_depth, options.surface_epsilon});

        //        RAYCHEL_ASSERT(result.hit_index != no_hit)
        if (result.hit_index == no_hit) {
            return color{0};
        }

        auto opposite_normal = get_normal(result.point, data.state.surfaces[result.hit_index], options.normal_epsilon);
        const auto opposite_shading_point = result.point + (opposite_normal * options.shading_epsilon);
        const auto out_direction = refract(
            trace_direction,
            opposite_normal,
            data.material_ior,
            get_surrounding_ior(data.surface_point, data.state.surfaces, data.state.materials));

        //Total internal reflection
        if (out_direction == vec3{}) {
            return get_shaded_color(RenderData{
                .origin = opposite_shading_point,
                .direction = reflect(trace_direction, opposite_normal),
                .state = data.state,
                .recursion_depth = data.recursion_depth});
        }

        return get_shaded_color(RenderData{
            .origin = opposite_shading_point,
            .direction = out_direction,
            .state = data.state,
            .recursion_depth = data.recursion_depth});
    }

    [[nodiscard]] static color get_refractive_component(const RefractionData& data, double refraction_factor) noexcept
    {
        if (refraction_factor < 0.01) {
            return color{};
        }

        const auto outer_ior = get_surrounding_ior(data.surface_point, data.state.surfaces, data.state.materials);
        if (data.ior_variation == 0.0) {
            return get_refractive_component(data, 1.0, outer_ior) * refraction_factor;
        }
        return color{
                   get_refractive_component(data, 1.0 - data.ior_variation, outer_ior).r(),
                   get_refractive_component(data, 1, outer_ior).g(),
                   get_refractive_component(data, 1.0 + data.ior_variation, outer_ior).b(),
               } *
               refraction_factor;
    }

    color get_refraction(const RefractionData& data) noexcept
    {
        const auto outer_ior = get_surrounding_ior(data.surface_point, data.state.surfaces, data.state.materials);

        const auto reflection_factor = fresnel(data.incoming_direction, data.normal, data.material_ior, outer_ior);

        return get_reflective_component(data, reflection_factor) + get_refractive_component(data, 1.0 - reflection_factor);
    }
} // namespace Raychel
