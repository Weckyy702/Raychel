/**
* \file Serialize.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for Serialize class
* \date 2022-05-19
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

#include "Raychel/Core/Serialize.h"
#include "Raychel/Core/Scene.h"

namespace Raychel {

    bool serialize_scene(const Scene& scene, std::ostream& os) noexcept
    {
        std::size_t i{};
        os << "--BEGIN SURFACES--\n";
        for (const auto& serializer : scene.object_serializers()) {
            if (!serializer.serialize(scene.objects().at(i), os))
                return false;
            ++i;
        }

        i = 0U;
        os << "--BEGIN MATERIALS--\n";
        for (const auto& serializer : scene.material_serializers()) {
            if (!serializer.serialize(scene.materials().at(i), os))
                return false;
            ++i;
        }

        return true;
    }

} //namespace Raychel
