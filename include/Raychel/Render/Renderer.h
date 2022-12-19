/**
* \file Renderer.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Renderer class
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
#ifndef RAYCHEL_RENDERER_H
#define RAYCHEL_RENDERER_H

#include "Camera.h"
#include "Framebuffer.h"
#include "MaterialContainer.h"
#include "Raychel/Core/SDFContainer.h"

#include <functional>
#include <vector>

namespace Raychel {

    struct RenderOptions
    {
        //Size of the output image
        Size2D output_size{1280, 720};

        //Maximum number of steps until raymarching terminates
        std::size_t max_ray_steps{1'024};

        //Maximum depth for recursive algorithms
        std::size_t max_recursion_depth{6};

        //Maximum number of light bounces for indirect lighting
        std::size_t max_lighting_bounces{2};

        //Number of samples per pixel for rendering. Dramatically increases render times!
        std::size_t samples_per_pixel{128};

        //If antialiasing is used. (Low performance impact)
        bool do_aa{true};

        //How many threads are used for rendering. If 0, the library will choose
        std::size_t thread_count{0};

        //Maximum distance a ray can travel
        double max_ray_depth{500};

        //Maximum distance between the ray and a surface
        double surface_epsilon{1e-6};
        //Radius used for normal calculation. Should be smaller than surface_epsilon to avoid weirdness
        double normal_epsilon{1e-12};
        //Offset along the surface normal to avoid shadow weirdness. Should be larger than surface_epsilon
        double shading_epsilon{1e-5};
    };

    struct RenderState
    {
        const std::vector<SDFContainer>& surfaces;
        const std::vector<MaterialContainer>& materials;
        BackgroundFunction get_background{};
        RenderOptions options{};
    };

    struct RenderData
    {
        vec3 origin, direction;

        const RenderState& state;
        std::size_t recursion_depth;
    };

    FatFramebuffer render_scene(const Scene& scene, const Camera& camera, const RenderOptions& options = {}) noexcept;

} // namespace Raychel

#endif //!RAYCHEL_RENDERER_H
