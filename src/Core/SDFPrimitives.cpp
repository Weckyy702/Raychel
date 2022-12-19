/**
* \file SDFPrimitives.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for SDFPrimitives class
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

#include "Raychel/Core/SDFPrimitives.h"

namespace Raychel {

    bool do_serialize(std::ostream& os, const Sphere& object) noexcept
    {
        os << object.radius << '\n';
        return os.good();
    }

    std::optional<Sphere> do_deserialize(std::istream& is, DeserializationTag<Sphere>) noexcept
    {
        double radius{};
        if (!(is >> radius))
            return std::nullopt;
        return Sphere{radius};
    }

    bool do_serialize(std::ostream& os, const Box& object) noexcept
    {
        os << object.size << '\n';
        return os.good();
    }

    std::optional<Box> do_deserialize(std::istream& is, DeserializationTag<Box> /*unused*/) noexcept
    {
        vec3 size{};
        if (!(is >> size))
            return std::nullopt;

        return Box{size};
    }

    bool do_serialize(std::ostream& os, const Plane& object) noexcept
    {
        os << object.normal << '\n';
        return os.good();
    }

    std::optional<Plane> do_deserialize(std::istream& is, DeserializationTag<Plane>) noexcept
    {
        vec3 normal{};
        if (!(is >> normal))
            return std::nullopt;
        if (normal == vec3{})
            return std::nullopt;

        return Plane{normalize(normal)};
    }
} //namespace Raychel
