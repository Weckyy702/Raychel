/**
* \file Renderer.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for Renderer class
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

#include "Raychel/Render/Renderer.h"
#include "Raychel/Core/Scene.h"
#include "Raychel/Core/ZigguratNormal.h"
#include "Raychel/Render/FatPixel.h"
#include "Raychel/Render/RenderUtils.h"

#include "RaychelCore/ScopedTimer.h"

#include <algorithm>
#include <atomic>
#include <execution>
#include <fstream>
#include <map>
#include <random>
#include <thread>

namespace Raychel {

    struct CacheComparator
    {
        constexpr bool operator()(const auto& lhs, const auto& rhs) const
        {
            if (lhs.first < rhs.first) {
                return true;
            }
            return std::lexicographical_compare(lhs.second.begin(), lhs.second.end(), rhs.second.begin(), rhs.second.end());
        }
    };

    struct CacheResult
    {
        std::vector<vec3>& data;
        bool data_was_in_cache{false};
    };

    static CacheResult get_cached_ray_data(const Camera& camera, const RenderOptions options) noexcept
    {
        using Key = std::pair<double, Size2D>;

        static std::map<Key, std::vector<vec3>, CacheComparator> ray_cache{};

        const Key cache_key{camera.zoom, options.output_size};

        if (const auto it = ray_cache.find(cache_key); it != ray_cache.end()) {
            Logger::debug("Found cached ray data for zoom=", camera.zoom, ", size=", options.output_size, '\n');
            return {it->second, true};
        }

        Logger::debug("Cache not populated. Generating rays for ", options.output_size, " plane\n");
        return {ray_cache[cache_key], false};
    }

    static void generate_rays_internal(std::vector<vec3>& rays, const Camera& camera, const RenderOptions options) noexcept
    {
        using vec2 = basic_vec2<double>;
        constexpr vec3 right{1, 0, 0};
        constexpr vec3 up{0, 1, 0};
        constexpr vec3 forward{0, 0, 1};

        const auto get_relative_coordinates = [&options](const vec2 pixel_coordinate) {
            const auto [plane_x, plane_y] = options.output_size;

            const auto aspect_ratio = static_cast<double>(plane_x) / static_cast<double>(plane_y);

            const auto raw_relative_x = pixel_coordinate.x() / static_cast<double>(plane_x) - 0.5;
            const auto raw_relative_y = pixel_coordinate.y() / static_cast<double>(plane_y) - 0.5;
            if (aspect_ratio > 1.0) {
                return vec2{raw_relative_x * aspect_ratio, raw_relative_y};
            }
            return vec2{raw_relative_x, raw_relative_y / aspect_ratio};
        };

        rays.reserve(options.output_size.y() * options.output_size.x());

        for (std::size_t y{options.output_size.y()}; y != 0U; --y) {
            for (std::size_t x{}; x != options.output_size.x(); ++x) {
                const auto [relative_x, relative_y] = get_relative_coordinates(vec2{x, y});

                // clang-format off
                const auto direction = normalize(   (right * relative_x) +
                                                    (up * relative_y)    +
                                                    (forward * camera.zoom));
                // clang-format on

                rays.emplace_back(direction);
            }
        }
    }

    static const std::vector<vec3>& generate_rays(const Camera& camera, const RenderOptions& options) noexcept
    {
        const auto [rays, was_cached] = get_cached_ray_data(camera, options);

        if (was_cached) {
            return rays;
        }

        generate_rays_internal(rays, camera, options);

        return rays;
    }

    [[nodiscard]] static vec3 get_direction_with_aa(const vec3& direction, const Size2D& output_size) noexcept
    {
        const vec3 jitter{
            uniform_random() / static_cast<double>(output_size.x()), uniform_random() / static_cast<double>(output_size.y())};

        return normalize(direction + jitter);
    }

    [[maybe_unused]] static void
    write_framebuffer(const std::string& file_name, Size2D size, const std::vector<FatPixel>& pixel_data) noexcept
    {
        std::ofstream output_image{file_name};
        if (!output_image) {
            return;
        }

        output_image << "P6\n" << size.x() << ' ' << size.y() << '\n' << "255\n";
        for (const auto& pixel : pixel_data) {
            const auto pixel_rgb = convert_color<std::uint8_t>(pixel.noisy_color);
            if (!output_image.write(reinterpret_cast<const char*>(&pixel_rgb), sizeof(pixel_rgb)).good()) {
                break;
            }
        }
    }

    [[nodiscard]] static std::vector<FatPixel>
    render_fat_pixels(const Scene& scene, const Camera& camera, const RenderOptions& options) noexcept
    {
        const auto& rays = generate_rays(camera, options);
        std::vector<FatPixel> fat_pixels{rays.size()};

        ScopedTimer<std::chrono::milliseconds> timer{"Render time"};

        std::atomic_size_t pixels_rendered{};

        std::jthread notifier{[&pixels_rendered, pixel_count = rays.size(), &fat_pixels, options] {
            using namespace std::chrono_literals;

            [[maybe_unused]] std::chrono::high_resolution_clock::time_point last_check_point{};
            std::size_t pixels_so_far{};
            do {
                const auto previous = pixels_so_far;
                pixels_so_far = pixels_rendered.load();

                const auto percentage = (pixels_so_far * 100) / pixel_count; //beware of overflow *spooky noises*
                const auto pixels_per_second = (pixels_so_far - previous) * 1'000 / 30;

                Logger::info(
                    "Rendered ",
                    pixels_so_far,
                    "/",
                    pixel_count,
                    " pixels (",
                    percentage,
                    "%) ~",
                    pixels_per_second,
                    " pixels per second              \r"); //big brain padding

#if 1
                const auto now = std::chrono::high_resolution_clock::now();
                if (duration_cast<std::chrono::seconds>(now-last_check_point).count() >= 1) {
                    last_check_point = now;
                    write_framebuffer("progress.ppm", options.output_size, fat_pixels);
                }
#endif
                std::this_thread::sleep_for(30ms);
            } while (pixels_so_far != pixel_count);
            Logger::log('\n');
        }};

        const RenderState state{scene.objects(), scene.materials(), scene.background_function(), options};

        std::transform(std::execution::par, rays.begin(), rays.end(), fat_pixels.begin(), [&](const vec3& ray_direction) {
            const auto get_direction = [&] {
                if (options.do_aa) {
                    return get_direction_with_aa(ray_direction, options.output_size) * camera.transform.rotation;
                }
                return ray_direction * camera.transform.rotation;
            };

            FatPixel::Histogram histogram;
            color pixel_color{};

            for (std::size_t i{}; i != options.samples_per_pixel; ++i) {
                const auto sample_direction = get_direction();
                const auto sample = get_shaded_color(RenderData{camera.transform.offset, sample_direction, state, 0U});
                histogram.add_sample(sample);
                pixel_color += (sample / options.samples_per_pixel);
            }

            ++pixels_rendered;

            return FatPixel{pixel_color, histogram};
        });

        return fat_pixels;
    }

    FatFramebuffer render_scene(const Scene& scene, const Camera& camera, const RenderOptions& options) noexcept
    {
        return {options.output_size, render_fat_pixels(scene, camera, options)};
    }

} //namespace Raychel
