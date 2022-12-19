/**
* \file Xoroshiro128+.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Xoroshiro128+ class
* \date 2022-04-24
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
#ifndef RAYCHEL_XOROSHIRO128_H
#define RAYCHEL_XOROSHIRO128_H

#include <bit>
#include <concepts>
#include <cstdint>
#include <limits>

namespace Raychel {

    //This is an implementation of the xoroshiro128+ algorithm as described at https://prng.di.unimi.it/xoroshiro128plus.c ported to C++20

    //    template<int A=24, int B=16, int C=37>

    //TODO: template this once we figure out how jump() and long_jump() can be generalized
    class Xoroshiro128
    {
        struct TwoUint64s
        {
            std::uint64_t first, second;
        };

    public:
        static constexpr std::int32_t A = 24, B = 16, C = 37;

        constexpr Xoroshiro128() = default;

        constexpr Xoroshiro128(std::uint64_t seed) : state_{seed, 0}
        {
            jump(); //jump through the domain a bit so we don't get weird results on the first call to next()
        }

        constexpr Xoroshiro128(std::uint64_t s0, std::uint64_t s1) : state_{s0, s1}
        {}

        static constexpr auto min() noexcept
        {
            return std::numeric_limits<std::uint64_t>::min();
        }

        static constexpr auto max() noexcept
        {
            return std::numeric_limits<std::uint64_t>::max();
        }

        template <std::integral Result = std::uint64_t>
        [[nodiscard]] constexpr Result operator()() noexcept
        {
            return next<Result>();
        }

        template <std::integral Result = std::uint64_t>
        constexpr Result next() noexcept
        {
            static_assert(sizeof(Result) <= sizeof(std::uint64_t), "Xoroshiro128+ cannot produce more than 64 random bits!");

            auto [s0, s1] = state_;
            const auto result = static_cast<Result>(s0 + s1);

            s1 ^= s0;
            state_.first = std::rotl(s0, A) ^ s1 ^ (s1 << B);
            state_.second = std::rotl(s1, C);

            return result;
        }

        constexpr void jump() noexcept
        {
            uint64_t s0{};
            uint64_t s1{};

            for (std::size_t b{}; b != 64U; ++b) {
                if (short_jump_.first & std::uint64_t{1} << b) {
                    s0 ^= state_.first;
                    s1 ^= state_.second;
                }
                next();
            }

            for (std::size_t b{}; b != 64U; ++b) {
                if (short_jump_.second & std::uint64_t{1} << b) {
                    s0 ^= state_.first;
                    s1 ^= state_.second;
                }
                next();
            }

            state_.first = s0;
            state_.second = s1;
        }

        constexpr void long_jump() noexcept
        {
            uint64_t s0{};
            uint64_t s1{};

            for (std::size_t b{}; b != 64; ++b) {
                if (long_jump_.first & std::uint64_t{1} << b) {
                    s0 ^= state_.first;
                    s1 ^= state_.second;
                }
                next();
            }

            for (std::size_t b{}; b != 64; ++b) {
                if (long_jump_.second & std::uint64_t{1} << b) {
                    s0 ^= state_.first;
                    s1 ^= state_.second;
                }
                next();
            }

            state_.first = s0;
            state_.second = s1;
        }

    private:
        TwoUint64s state_{123456789, 987654321};

        static constexpr TwoUint64s short_jump_{0xdf900294d8f554a5, 0x170865df4b3201fc};
        static constexpr TwoUint64s long_jump_{0xd2a98b26625eee7b, 0xdddf9b1090aa7ac1};
    };

} // namespace Raychel

#endif //!RAYCHEL_XOROSHIRO128_H
