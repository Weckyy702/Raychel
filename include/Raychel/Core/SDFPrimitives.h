/**
* \file SDFPrimitives.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for SDF primitives
* \date 2022-05-27
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
#ifndef RAYCHEL_SDF_PRIMITIVES_H
#define RAYCHEL_SDF_PRIMITIVES_H

#include "Types.h"

#include <cmath>
#include <iostream>
#include <optional>

namespace Raychel {

    struct Sphere
    {
        double radius{1.0};
    };

    inline double evaluate_sdf(const Sphere& object, const vec3& p) noexcept
    {
        return mag(p) - object.radius;
    }

    //optimized normal calculation for spheres
    inline vec3 evaluate_normal(const Sphere& /*unused*/, const vec3& p) noexcept
    {
        return normalize(p);
    }

    bool do_serialize(std::ostream& os, const Sphere& object) noexcept;

    std::optional<Sphere> do_deserialize(std::istream& is, DeserializationTag<Sphere>) noexcept;

    struct Box
    {
        vec3 size{1, 1, 1};
    };

    inline double evaluate_sdf(const Box& box, const vec3& p) noexcept
    {
        using std::abs, std::max, std::min;
        const auto q = vec3{abs(p.x()), abs(p.y()), abs(p.z())} - box.size;

        return mag(vec3{max(q.x(), 0.0), max(q.y(), 0.0), max(q.z(), 0.0)}) + min(max(q.x(), max(q.y(), q.z())), 0.0);
    }

    bool do_serialize(std::ostream& os, const Box& object) noexcept;

    std::optional<Box> do_deserialize(std::istream& is, DeserializationTag<Box>) noexcept;

    struct Plane
    {
        vec3 normal{0, 1, 0};
    };

    inline double evaluate_sdf(const Plane& object, const vec3& p) noexcept
    {
        return std::abs(dot(object.normal, p));
    }

    inline vec3 evaluate_normal(const Plane& object, const vec3&) noexcept
    {
        return object.normal;
    }

    bool do_serialize(std::ostream& os, const Plane& object) noexcept;

    std::optional<Plane> do_deserialize(std::istream& is, DeserializationTag<Plane>) noexcept;

    struct DeserializationErrorPlaceHolder
    {};

    inline double evaluate_sdf(const DeserializationErrorPlaceHolder /*unused*/, const vec3& /*unused*/) noexcept
    {
        return 1e9;
    }

} // namespace Raychel

#endif //!RAYCHEL_SDF_PRIMITIVES_H
