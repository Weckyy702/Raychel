/**
* \file Framebuffer.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Framebuffer class
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
#ifndef RAYCHEL_FRAMEBUFFER_H
#define RAYCHEL_FRAMEBUFFER_H

#include "FatPixel.h"
#include "Raychel/Core/Types.h"

#include <vector>

namespace Raychel {

    namespace details {
        template <typename Pixel>
        struct BasicFramebuffer
        {

            constexpr Pixel& at(std::size_t x, std::size_t y)
            {
                return const_cast<Pixel&>(const_cast<const BasicFramebuffer*>(this)->at(x, y));
            }

            constexpr const Pixel& at(std::size_t x, std::size_t y) const
            {
                RAYCHEL_ASSERT((x < size.x()) && (y < size.y()));

                return pixel_data.at(x + (y * size.x()));
            }

            const Size2D size{};
            std::vector<Pixel> pixel_data{};
        };

        template <typename T>
        constexpr auto begin(const BasicFramebuffer<T>& obj)
        {
            return obj.pixel_data.begin();
        }

        template <typename T>
        constexpr auto end(const BasicFramebuffer<T>& obj)
        {
            return obj.pixel_data.end();
        }
    } // namespace details

    using Framebuffer = details::BasicFramebuffer<color>;
    using FatFramebuffer = details::BasicFramebuffer<FatPixel>;

} // namespace Raychel

#endif //!RAYCHEL_FRAMEBUFFER_H
