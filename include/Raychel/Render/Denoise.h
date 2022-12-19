/**
* \file Denoise.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Denoise class
* \date 2022-05-06
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
#ifndef RAYCHEL_DENOISE_H
#define RAYCHEL_DENOISE_H

#include "FatPixel.h"
#include "Framebuffer.h"

namespace Raychel {

    struct DenoisingOptions
    {
        std::size_t half_patch_size{1};
        std::size_t half_search_window_size{6};
        double distance_threshold{1.};
        std::size_t num_scales{3};
    };

    Framebuffer denoise_single_scale(const FatFramebuffer& input_pixels, DenoisingOptions options = {}) noexcept;

    Framebuffer denoise_multiscale(const FatFramebuffer& input_pixels, DenoisingOptions options = {}) noexcept;

} // namespace Raychel

#endif //!RAYCHEL_DENOISE_H
