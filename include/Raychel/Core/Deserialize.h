/**
* \file Deserializer.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for Deserializer class
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
#ifndef RAYCHEL_DESERIALIZER_H
#define RAYCHEL_DESERIALIZER_H

#include <istream>
#include <optional>
#include <string_view>

#include "SDFTransforms.h"
#include "Scene.h"

namespace Raychel {

    namespace details {

        template <typename T>
        concept DeserializableWithoutTarget = requires()
        {
            // clang-format off
            requires !has_target_v<T>;
            { do_deserialize(std::declval<std::istream&>(), DeserializationTag<T>{}) } -> std::same_as<std::optional<T>>;
            // clang-format on
        };

        template <typename T, typename Container>
        concept DeserializableWithTarget = requires()
        {
            // clang-format off
            requires has_target_v<T>;
            { do_deserialize(std::declval<std::istream&>(), std::declval<Container>(), DeserializationTag<T>{}) } -> std::same_as<std::optional<T>>;
            // clang-format on
        };

        template <typename T, typename Container>
        concept Deserializable = DeserializableWithoutTarget<T> || DeserializableWithTarget<T, Container>;

        template <typename Container>
        struct IDeserializer
        {
            IDeserializer() = default;

            RAYCHEL_MAKE_NONCOPY_NONMOVE(IDeserializer)

            virtual std::optional<Container>
            deserialize(std::istream& is, std::optional<Container>&& maybe_target) const noexcept = 0;

            virtual SerializableObjectData<Container> get_serializer() const noexcept = 0;

            virtual std::string_view contained_type_name() const noexcept = 0;

            virtual ~IDeserializer() = default;
        };

        template <typename Container, Deserializable<Container> Object>
        class Deserializer final : public IDeserializer<Container>
        {
        public:
            Deserializer() = default;

            std::optional<Container> deserialize(std::istream& is, std::optional<Container>&& maybe_target) const noexcept final
            {
                if constexpr (has_target_v<Object>) {
                    return _deserialize_with_target(is, std::move(maybe_target));
                } else {
                    if (maybe_target.has_value()) {
                        Logger::warn("Type ", Logger::details::type_name<Object>(), " did not expect to have a target!\n");
                        return std::nullopt;
                    }
                    return _deserialize_without_target(is);
                }
            }

            SerializableObjectData<Container> get_serializer() const noexcept final
            {
                return SerializableObjectData<Container>{SerializableObjectDescriptor<Object>{}};
            }

            std::string_view contained_type_name() const noexcept final
            {
                return serializable_type_name_for<Object>();
            }

            virtual ~Deserializer() = default;

        private:
            static std::optional<Container> _deserialize_without_target(std::istream& is) noexcept
            {
                auto maybe_object = do_deserialize(is, DeserializationTag<Object>{});
                if (!maybe_object.has_value())
                    return std::nullopt;
                return Container{std::move(maybe_object).value()};
            }

            static std::optional<Container>
            _deserialize_with_target(std::istream& is, std::optional<Container> maybe_target) noexcept
            {
                if (!maybe_target.has_value()) {
                    Logger::warn("Type ", Logger::details::type_name<Object>(), " expectec to have a target!\n");
                    return std::nullopt;
                }
                auto maybe_object = do_deserialize(is, std::move(maybe_target).value(), DeserializationTag<Object>{});
                if (!maybe_object.has_value())
                    return std::nullopt;
                return Container{std::move(maybe_object).value()};
            }
        };

        template <typename Container>
        using DeserializerPtr = std::unique_ptr<IDeserializer<Container>>;

        template <typename Container, typename... Args>
        auto object_deserializers()
        {
            std::vector<DeserializerPtr<Container>> res{};

            ((res.emplace_back(std::make_unique<Deserializer<Container, Args>>())), ...);

            return res;
        }
    } // namespace details

    template <typename... Objects>
    auto object_deserializers()
    {
        return details::object_deserializers<SDFContainer, Objects...>();
    }

    template <typename... Materials>
    auto material_deserializers()
    {
        return details::object_deserializers<MaterialContainer, Materials...>();
    }

    [[nodiscard]] Scene deserialize_scene(
        std::istream& is, const std::vector<details::DeserializerPtr<SDFContainer>>& object_deserializers,
        const std::vector<details::DeserializerPtr<MaterialContainer>>& material_deserializers) noexcept;

} //namespace Raychel

#endif //!RAYCHEL_DESERIALIZER_H
