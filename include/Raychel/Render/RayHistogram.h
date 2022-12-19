/**
* \file RayHistogram.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for RayHistogram class
* \date 2022-04-30
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
#ifndef RAYCHEL_RAY_HISTOGRAM_H
#define RAYCHEL_RAY_HISTOGRAM_H

#include "Raychel/Core/Types.h"

#include <array>
#include <cstdint>

namespace Raychel {

    namespace details {

        struct BinData
        {
            std::size_t low_bin_index{};
            double low_bin_weight{};

            std::size_t high_bin_index{};
            double high_bin_weight{};
        };
    }; // namespace details

    template <std::size_t NumBins>
    requires(NumBins > 2U) class RayHistogram
    {
        using BinnedChannel = std::array<double, NumBins>;

    public:
        RayHistogram() = default;

        void add_sample(color c) noexcept
        {
            _add_channel(red_, c.r());
            _add_channel(green_, c.g());
            _add_channel(blue_, c.b());
        }

        const auto& red_channel() const noexcept
        {
            return red_;
        }

        const auto& green_channel() const noexcept
        {
            return green_;
        }

        const auto& blue_channel() const noexcept
        {
            return blue_;
        }

        RayHistogram operator+(const RayHistogram& other) const noexcept
        {
            BinnedChannel union_red{}, union_green{}, union_blue{};

            for (std::size_t i{}; i != NumBins; ++i) {
                union_red[i] = red_[i] + other.red_[i];
                union_green[i] = green_[i] + other.green_[i];
                union_blue[i] = blue_[i] + other.blue_[i];
            }

            return {std::move(union_red), std::move(union_green), std::move(union_blue)};
        }

        template <std::convertible_to<double> T>
        RayHistogram operator/(T _s) const noexcept
        {
            const auto s = static_cast<double>(_s);

            BinnedChannel new_red{}, new_green{}, new_blue{};
            for (std::size_t i{}; i != NumBins; ++i) {
                new_red[i] = red_[i] / s;
                new_green[i] = green_[i] / s;
                new_blue[i] = blue_[i] / s;
            }

            return {std::move(new_red), std::move(new_green), std::move(new_blue)};
        }

    private:
        RayHistogram(BinnedChannel red, BinnedChannel green, BinnedChannel blue)
            : red_{std::move(red)}, green_{std::move(green)}, blue_{std::move(blue)}
        {}

        static void _add_channel(BinnedChannel& channel, double value) noexcept
        {
            const auto [low_bin, low_weight, high_bin, high_weight] = _get_bin_data(value);
            channel.at(low_bin) += low_weight;
            channel.at(high_bin) += high_weight;
        }

        static details::BinData _get_bin_data(double value) noexcept
        {
            constexpr auto max_value = 7.5;
            constexpr auto saturated_value = 2.5;
            constexpr auto fbin_factor = static_cast<double>(NumBins - 2U);

            auto v = std::max(value, 0.0);
            v = std::pow(v, 1.0 / 2.2);
            v /= max_value;

            v = std::min(saturated_value, v);

            const auto fbin = v * fbin_factor;

            const auto bin_low = static_cast<std::size_t>(fbin);
            if (bin_low < (NumBins - 2U)) {
                const auto high_weight = std::fmod(fbin, 1.0);
                const auto low_weight = 1.0 - high_weight;

                return {bin_low, low_weight, bin_low + 1U, high_weight};
            }
            const auto high_weight = (v - 1) / (saturated_value - 1);
            const auto low_weight = 1.0 - high_weight;

            return {NumBins - 2U, low_weight, NumBins - 1U, high_weight};
        }

        BinnedChannel red_{};
        BinnedChannel green_{};
        BinnedChannel blue_{};
    };
} // namespace Raychel

#endif //!RAYCHEL_RAY_HISTOGRAM_H
