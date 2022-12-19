/**
* \file SDFTransforms.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for SDFTransforms class
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
#ifndef RAYCHEL_SDF_TRANSFORM_H
#define RAYCHEL_SDF_TRANSFORM_H

#include "Types.h"
#include "SDFContainer.h"

#include <iostream>
#include <optional>

namespace Raychel {

    template <typename Target = SDFContainer>
    struct Translate
    {
        Target target;
        vec3 translation{};
    };

    template <typename T>
    struct has_target<Translate<T>> : std::true_type
    {};

    template <typename T>
    Translate(T, vec3) -> Translate<T>;

    template <typename T>
    double evaluate_sdf(const Translate<T>& object, const vec3& p) noexcept
    {
        return evaluate_sdf(object.target, p - object.translation);
    }

    template <typename T>
    bool do_serialize(std::ostream& os, const Translate<T>& object) noexcept
    {
        os << object.translation << '\n';
        return os.good();
    }

    template <typename T>
    std::optional<Translate<T>> do_deserialize(std::istream& is, SDFContainer target, DeserializationTag<Translate<T>>) noexcept
    {
        vec3 translation{};

        if (!(is >> translation))
            return std::nullopt;
        return Translate<T>{std::move(target), translation};
    }

    template <typename Target = SDFContainer>
    struct Rotate
    {
        Target target;
        Quaternion rotation{};
    };

    template <typename T>
    struct has_target<Rotate<T>> : std::true_type
    {};

    template <typename T>
    Rotate(T, Quaternion) -> Rotate<T>;

    template <typename T>
    double evaluate_sdf(const Rotate<T>& object, const vec3& p) noexcept
    {
        return evaluate_sdf(object.target, p * inverse(object.rotation));
    }

    template <typename T>
    bool do_serialize(std::ostream& os, const Rotate<T>& object) noexcept
    {
        os << object.rotation << '\n';
        return os.good();
    }

    template <typename T>
    std::optional<Rotate<T>> do_deserialize(std::istream& is, SDFContainer target, DeserializationTag<Rotate<T>>) noexcept
    {
        Quaternion rotation{};

        if (!(is >> rotation))
            return std::nullopt;
        return Rotate{std::move(target), rotation};
    }

} // namespace Raychel

#endif //!RAYCHEL_SDF_TRANSFORM_H
