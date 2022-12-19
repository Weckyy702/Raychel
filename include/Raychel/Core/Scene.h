/**
 * \file Scene.h
 * \author Weckyy702 (weckyy702@gmail.com)
 * \brief Header file for Scene class
 * \date 2022-04-09
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
#ifndef RAYCHEL_SCENE_H
#define RAYCHEL_SCENE_H

#include "Raychel/Render/MaterialContainer.h"
#include "SDFContainer.h"
#include "Serialize.h"

#include <algorithm>
#include <iterator>
#include <vector>

namespace Raychel {

    template <typename Object, typename Material>
    struct RaymarchableObject
    {
        std::size_t index_in_scene;
        Object& object;
        Material& material;
    };

    //Clang needs this deduction guide
    template <typename Object, typename Material>
    RaymarchableObject(std::size_t, Object&, Material&) -> RaymarchableObject<Object, Material>;

    class Scene
    {
    public:
        Scene() = default;

        static Scene unsafe_from_data(
            std::vector<SDFContainer> objects, std::vector<SerializableObjectData<SDFContainer>> object_serializers,
            std::vector<MaterialContainer> materials,
            std::vector<SerializableObjectData<MaterialContainer>> material_serializers) noexcept;

        template <typename Object, typename Material>
        auto add_object(Object&& object, Material&& material) noexcept
        {
            const auto where = std::lower_bound(
                objects_.begin(), objects_.end(), details::TypeId<Object>::id(), [](const auto& cont, const auto& id) {
                    return cont.type_id() < id;
                });

            const auto obj = objects_.emplace(where, std::forward<Object>(object));
            const auto index = std::distance(objects_.begin(), obj);

            const auto mat = materials_.emplace(materials_.begin() + index, std::forward<Material>(material));

            object_serializers_.emplace(object_serializers_.begin() + index, details::SerializableObjectDescriptor<Object>{});
            material_serializers_.emplace(
                material_serializers_.begin() + index, details::SerializableObjectDescriptor<Material>{});

            return RaymarchableObject{
                objects_.size() - 1U,
                details::get_container_content<Object>(*obj),
                details::get_container_content<Material>(*mat)};
        }

        void remove_object(std::size_t index) noexcept;

        template <std::invocable<const RenderData&> F>
        requires(std::is_same_v<std::invoke_result_t<F, const RenderData&>, color>) void set_background_function(F&& f) noexcept
        {
            background_function_ = f;
        }

        [[nodiscard]] const auto& objects() const noexcept
        {
            return objects_;
        }

        [[nodiscard]] const auto& materials() const noexcept
        {
            return materials_;
        }

        [[nodiscard]] const auto& background_function() const noexcept
        {
            return background_function_;
        }

        [[nodiscard]] const auto& object_serializers() const noexcept
        {
            return object_serializers_;
        }

        [[nodiscard]] const auto& material_serializers() const noexcept
        {
            return material_serializers_;
        }

    private:
        Scene(
            std::vector<SDFContainer> objects, std::vector<SerializableObjectData<SDFContainer>> object_serializers,
            std::vector<MaterialContainer> materials, std::vector<SerializableObjectData<MaterialContainer>> material_serializers)
            : object_serializers_{std::move(object_serializers)},
              material_serializers_{std::move(material_serializers)},
              objects_{std::move(objects)},
              materials_{std::move(materials)}
        {}

        std::vector<SerializableObjectData<SDFContainer>> object_serializers_{};
        std::vector<SerializableObjectData<MaterialContainer>> material_serializers_{};

        std::vector<SDFContainer> objects_{};
        std::vector<MaterialContainer> materials_{};
        BackgroundFunction background_function_{};
    };

    bool serialize_scene(const Scene& scene, std::ostream& os) noexcept;

} // namespace Raychel

#endif //! RAYCHEL_SCENE_H
