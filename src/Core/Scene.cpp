/**
* \file Scene.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for Scene class
* \date 2022-05-17
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

#include "Raychel/Core/Scene.h"

namespace Raychel {

    void Scene::remove_object(std::size_t _index) noexcept
    {
        using Diff = std::iter_difference_t<decltype(objects_.begin())>;
        if (_index >= objects_.size()) {
            return;
        }

        RAYCHEL_ASSERT(_index < std::numeric_limits<Diff>::max())

        const auto index = static_cast<Diff>(_index);

        objects_.erase(objects_.begin() + index);
        materials_.erase(materials_.begin() + index);
        object_serializers_.erase(object_serializers_.begin() + index);
        material_serializers_.erase(material_serializers_.begin() + index);
    }

    Scene Scene::unsafe_from_data(
        std::vector<SDFContainer> objects, std::vector<SerializableObjectData<SDFContainer>> object_serializers,
        std::vector<MaterialContainer> materials,
        std::vector<SerializableObjectData<MaterialContainer>> material_serializers) noexcept
    {
        //These all need to be the exact same size
        if (!(objects.size() == object_serializers.size() && objects.size() == materials.size() &&
              objects.size() == material_serializers.size())) {
            Logger::warn(
                "Unable to create Scene from invalid data! Data sizes:"
                "\n Objects: ",
                objects.size(),
                "\n Object serializers: ",
                object_serializers.size(),
                "\n Materials: ",
                materials.size(),
                "\n Material serializers: ",
                material_serializers.size(),
                '\n');
            return Scene{};
        }

        return Scene{std::move(objects), std::move(object_serializers), std::move(materials), std::move(material_serializers)};
    }

} //namespace Raychel
