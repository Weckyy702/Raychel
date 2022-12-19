/**
* \file Denoise.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation of the Ray Histogram Fusion algorithm by Mauricio Delbracio et al.
* \date 2022-05-02
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

#include "Raychel/Render/Denoise.h"

#include <cmath>
#include <functional>
#include <thread>

namespace Raychel {

    static std::size_t to_index(std::size_t x, std::size_t y, std::size_t image_width)
    {
        return x + (y * image_width);
    }

    static std::size_t safe_sub(std::size_t a, std::size_t b)
    {
        if (b > a) {
            return 0U;
        }
        return a - b;
    }

    template <std::size_t NumBins>
    static double chi_squared_distance(const std::array<double, NumBins>& a, const std::array<double, NumBins>& b)
    {
        double sum{};
        double num_nonempty_bins{};

        for (std::size_t i{}; i != NumBins; ++i) {
            const auto divisor = a[i] + b[i];
            if (divisor != 0.0) {
                sum += sq(a[i] - b[i]) / divisor;
                ++num_nonempty_bins;
            }
        }

        if (num_nonempty_bins == 0.0) {
            return 0.0;
        }

        return sum / num_nonempty_bins;
    }

    template <std::size_t NumBins>
    static auto chi_squared_distance(const RayHistogram<NumBins>& a, const RayHistogram<NumBins>& b)
    {
        //TODO: process the three channels in one loop
        return Tuple{
            chi_squared_distance(a.red_channel(), b.red_channel()),
            chi_squared_distance(a.green_channel(), b.green_channel()),
            chi_squared_distance(a.blue_channel(), b.blue_channel())};
    }

    struct SearchWindow
    {
        [[nodiscard]] std::size_t width() const noexcept
        {
            return end_x - start_x;
        }

        [[nodiscard]] std::size_t height() const noexcept
        {
            return end_y - start_y;
        }

        [[nodiscard]] std::size_t area() const noexcept
        {
            return width() * height();
        }

        std::size_t start_x{}, start_y{}, end_x{}, end_y{};
    };

    using Patch = SearchWindow;

    static SearchWindow
    search_window_for_pixel(std::size_t x, std::size_t y, Size2D image_size, std::size_t half_search_window_size) noexcept
    {
        const auto start_x = safe_sub(x, half_search_window_size);
        const auto start_y = safe_sub(y, half_search_window_size);

        const auto end_x = std::min(x + half_search_window_size, image_size.x());
        const auto end_y = std::min(y + half_search_window_size, image_size.y());

        return {start_x, start_y, end_x, end_y};
    }

    static Patch patch_for_pixel(std::size_t x, std::size_t y, Size2D image_size, std::size_t half_patch_size) noexcept
    {
        return search_window_for_pixel(x, y, image_size, half_patch_size);
    }

    static std::vector<color> get_denoised_patch_with_search_window(
        const SearchWindow& search_window, const Patch& this_patch, const std::vector<FatPixel>& input_pixels,
        const Size2D image_size, const DenoisingOptions& options) noexcept
    {
        std::vector<Tuple<double, 3>> c{this_patch.area()};
        std::vector<color> V(this_patch.area());

        for (auto search_y = search_window.start_y; search_y != search_window.end_y; ++search_y) {
            for (auto search_x = search_window.start_x; search_x != search_window.end_x; ++search_x) {

                const auto& other_patch = patch_for_pixel(search_x, search_y, image_size, options.half_patch_size);

                for (auto this_y = this_patch.start_y; this_y != this_patch.end_y; ++this_y) {
                    for (auto this_x = this_patch.start_x; this_x != this_patch.end_x; ++this_x) {
                        const auto index_in_image = to_index(this_x, this_y, image_size.x());
                        const auto& this_pixel = input_pixels.at(index_in_image);
                        const auto index_in_patch =
                            to_index(this_x - this_patch.start_x, this_y - this_patch.start_y, this_patch.width());

                        for (auto other_y = other_patch.start_y; other_y != other_patch.end_y; ++other_y) {
                            for (auto other_x = other_patch.start_x; other_x != other_patch.end_x; ++other_x) {

                                const auto& other_pixel = input_pixels.at(to_index(other_x, other_y, image_size.x()));
                                const auto d = chi_squared_distance(this_pixel.histogram, other_pixel.histogram);

                                for (std::size_t i{}; i != 3U; ++i) {
                                    if (d[i] < options.distance_threshold) {
                                        V.at(index_in_patch)[i] += other_pixel.noisy_color[i];
                                        ++c.at(index_in_patch)[i];
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        for (std::size_t i{}; i != V.size(); ++i) {
            for (std::size_t j{}; j != 3U; ++j) {
                if (c.at(i)[j] != 0) {
                    V.at(i)[j] /= c.at(i)[j];
                }
            }
        }
        return V;
    }

    static void denoise_part(
        std::vector<color>& output, Size2D begin, Size2D end, const std::vector<FatPixel>& input_pixels, Size2D image_size,
        DenoisingOptions options) noexcept
    {
        std::vector<double> N(input_pixels.size());

        for (auto y = begin.y(); y != end.y(); ++y) {
            for (auto x = begin.x(); x != end.x(); ++x) {
                const auto this_patch = patch_for_pixel(x, y, image_size, options.half_patch_size);
                const auto search_window = search_window_for_pixel(x, y, image_size, options.half_search_window_size);

                const auto V =
                    get_denoised_patch_with_search_window(search_window, this_patch, input_pixels, image_size, options);

                for (auto patch_y = this_patch.start_y; patch_y != this_patch.end_y; ++patch_y) {
                    for (auto patch_x = this_patch.start_x; patch_x != this_patch.end_x; ++patch_x) {
                        const auto index_in_patch =
                            to_index(patch_x - this_patch.start_x, patch_y - this_patch.start_y, this_patch.width());
                        const auto index_in_image = to_index(patch_x, patch_y, image_size.x());

                        ++N.at(index_in_image);

                        auto& p = output.at(index_in_image);
                        p += (V.at(index_in_patch) - p) / N.at(index_in_image);
                    }
                }
            }
        }
    }

    template <
        typename Pixel, std::invocable<Pixel, Pixel> Add = std::plus<Pixel>,
        std::invocable<Pixel, std::size_t> Divide = std::divides<void>>
    details::BasicFramebuffer<Pixel> gaussian_subsample(
        const details::BasicFramebuffer<Pixel>& input_pixels, std::size_t scale, Add&& adder = {}, Divide&& divider = {})
    {
        //Because we use operator<<, specifying a scale larger than the number of bits in an unsigned int would result in UB.
        RAYCHEL_ASSERT(scale < (sizeof(unsigned int) * 8));

        if (scale == 0U) {
            return input_pixels;
        }

        const auto half_sample_window_size = (1U << (scale - 1));
        const auto scaling_factor = std::pow(0.5, static_cast<double>(scale));
        const auto pixel_step = 1U << scale;
        const auto scaled_size = input_pixels.size * scaling_factor;

        if (scaled_size == Size2D{}) {
            return {};
        }

        std::vector<Pixel> output_pixels{scaled_size.x() * scaled_size.y(), Pixel{}};

        for (std::size_t y{}; y != scaled_size.y(); ++y) {
            for (std::size_t x{}; x != scaled_size.x(); ++x) {
                const auto sample_patch =
                    patch_for_pixel(x * pixel_step, y * pixel_step, input_pixels.size, half_sample_window_size);

                std::size_t num_samples{};
                Pixel output_pixel{};

                for (auto patch_y = sample_patch.start_y; patch_y != sample_patch.end_y; ++patch_y) {
                    for (auto patch_x = sample_patch.start_x; patch_x != sample_patch.end_x; ++patch_x) {
                        ++num_samples;
                        const auto& sample_pixel = input_pixels.pixel_data.at(to_index(patch_x, patch_y, input_pixels.size.x()));

                        output_pixel = adder(output_pixel, sample_pixel);
                    }
                }
                output_pixel = divider(output_pixel, num_samples);

                output_pixels.at(to_index(x, y, scaled_size.x())) = output_pixel;
            }
        }

        return {scaled_size, output_pixels};
    }

    static void add_scaled_part(
        const Raychel::Framebuffer& scaled_input, std::vector<Raychel::color>& output, std::size_t scale, std::size_t num_scales)
    {
        using namespace Raychel;
        RAYCHEL_ASSERT(scale < (sizeof(unsigned int) * 8));

        if (scale == 0U) {
            std::copy(scaled_input.pixel_data.begin(), scaled_input.pixel_data.end(), output.begin());
            return;
        }

        const auto scaling_factor = 1U << scale;
        const auto step_size = std::pow(0.5, static_cast<double>(scale));
        const auto correction_factor = 0.5 / static_cast<double>(num_scales);

        for (std::size_t y{}; y != scaled_input.size.y(); ++y) {
            for (std::size_t x{}; x != scaled_input.size.x(); ++x) {
                const auto next_x = std::min(x + 1U, scaled_input.size.x() - 1U);
                const auto next_y = std::min(y + 1U, scaled_input.size.y() - 1U);

                const auto top_left = scaled_input.at(x, y);
                const auto top_right = scaled_input.at(next_x, y);
                const auto bottom_left = scaled_input.at(x, next_y);
                const auto bottom_right = scaled_input.at(next_x, next_y);

                const auto large_x = x * scaling_factor;
                const auto large_y = y * scaling_factor;

                for (std::size_t part_y{}; part_y != scaling_factor; ++part_y) {
                    for (std::size_t part_x{}; part_x != scaling_factor; ++part_x) {
                        const auto relative_x = static_cast<double>(part_x) * step_size;
                        const auto relative_y = static_cast<double>(part_y) * step_size;

                        const auto right_weight = std::fmod(relative_x * static_cast<double>(scaling_factor), 1.0);
                        const auto left_weight = 1.0 - right_weight;
                        const auto bottom_weight = std::fmod(relative_y * static_cast<double>(scaling_factor), 1.0);
                        const auto top_weight = 1.0 - bottom_weight;

                        // clang-format off
                    output.at(to_index(large_x + part_x, large_y + part_y, scaled_input.size.x() * scaling_factor)) += (
                        (top_left * (top_weight * left_weight)) +
                        (top_right * (top_weight * right_weight)) +
                        (bottom_left * (top_weight * left_weight)) +
                        (bottom_right * (bottom_weight * right_weight))
                    ) * correction_factor;
                        // clang-format on
                    }
                }
            }
        }
    }

    static void denoise_threaded(
        std::vector<color>& output, const FatFramebuffer& input_pixels, unsigned int num_threads,
        DenoisingOptions options) noexcept
    {
        constexpr Size2D patch_size{128, 128};

        std::vector<std::jthread> thread_pool{};
        thread_pool.reserve(num_threads);

        std::atomic_size_t current_patch_index{0U};

        const auto num_patches_x =
            static_cast<std::size_t>(std::ceil(static_cast<double>(input_pixels.size.x()) / static_cast<double>(patch_size.x())));
        const auto num_patches_y =
            static_cast<std::size_t>(std::ceil(static_cast<double>(input_pixels.size.y()) / static_cast<double>(patch_size.y())));
        const auto max_patch_index = num_patches_x * num_patches_y - 1U;

        for (std::size_t i{}; i != num_threads; ++i) {
            thread_pool.emplace_back([&, i] {
                std::size_t patch_index{};
                do {
                    patch_index = current_patch_index.fetch_add(1U);

                    const auto x_index = patch_index % num_patches_x;
                    const auto y_index = patch_index / num_patches_x;

                    const Size2D patch_begin{
                        std::min(x_index * patch_size.x(), input_pixels.size.x()),
                        std::min(y_index * patch_size.y(), input_pixels.size.y())};
                    const Size2D patch_end{
                        std::min(patch_begin.x() + patch_size.x(), input_pixels.size.x()),
                        std::min(patch_begin.y() + patch_size.y(), input_pixels.size.y())};

                    Logger::debug("Thread ", i, " got patch ", patch_index, " from ", patch_begin, " to ", patch_end, '\n');

                    denoise_part(output, patch_begin, patch_end, input_pixels.pixel_data, input_pixels.size, options);
                } while (patch_index < max_patch_index);
            });
        }
    }

    void denoise_internal(std::vector<color>& output, const FatFramebuffer& input_pixels, DenoisingOptions options) noexcept
    {
        const auto num_threads = std::thread::hardware_concurrency();

        if (num_threads == 0U) {
            denoise_part(output, Size2D{}, input_pixels.size, input_pixels.pixel_data, input_pixels.size, options);
            return;
        }

        denoise_threaded(output, input_pixels, num_threads, options);
    }

    Framebuffer denoise_single_scale(const FatFramebuffer& input_pixels, DenoisingOptions options) noexcept
    {
        std::vector<color> output(input_pixels.pixel_data.size());

        denoise_internal(output, input_pixels, options);

        return {input_pixels.size, std::move(output)};
    }

    Framebuffer denoise_multiscale(const FatFramebuffer& input_pixels, DenoisingOptions options) noexcept
    {
        if (options.num_scales == 1U) {
            return denoise_single_scale(input_pixels, options);
        }

        RAYCHEL_TODO("Multiscale denoising");

        auto scale = options.num_scales - 1U;

        std::vector<color> u_old{};

        while (scale != 0) {
            //uS = Ds(u)
            const auto scaled_input = gaussian_subsample(input_pixels, scale);
            //TODO: verify that the number of samples is still the same

            std::vector<color> scaled_output{scaled_input.pixel_data.size(), color{}};
            denoise_internal(scaled_output, scaled_input, options);

            add_scaled_part({input_pixels.size, scaled_output}, u_old, scale, options.num_scales);

            --scale;
        }

        return {input_pixels.size, std::move(u_old)};
    }

} // namespace Raychel
