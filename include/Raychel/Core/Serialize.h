/**
* \file SerializableObjectData.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for SerializableObjectData class
* \date 2022-05-18
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
#ifndef RAYCHEL_SERIALIZABLE_OBJECT_DATA_H
#define RAYCHEL_SERIALIZABLE_OBJECT_DATA_H

#include "Types.h"

#include "RaychelCore/ClassMacros.h"
#include "RaychelCore/Raychel_assert.h"
#include "RaychelLogger/Logger.h"

#include <concepts>
#include <functional>
#include <ostream>

namespace Raychel {

    namespace details {

        template <typename Contained, typename Container>
        auto& get_container_content(Container& container) noexcept
        {
            using Impl = typename Container::template Impl<Contained>;

            auto* impl_interface = container.unsafe_impl();
            auto* impl = dynamic_cast<Impl*>(impl_interface);
            RAYCHEL_ASSERT(impl != nullptr);

            return impl->object();
        }

        template <typename Contained, typename Container>
        const auto& get_container_content(const Container& container) noexcept
        {
            using Impl = typename Container::template Impl<Contained>;

            auto* impl_interface = container.unsafe_impl();
            auto* impl = dynamic_cast<Impl*>(impl_interface);
            RAYCHEL_ASSERT(impl != nullptr);
            return impl->object();
        }

        template <typename T>
        requires(has_target_v<T>) constexpr std::string_view serializable_type_name_for()
        {
            constexpr auto full_type_name = Logger::details::type_name<T>();
            constexpr auto bracket_index = full_type_name.find('<');
            static_assert(bracket_index != std::string_view::npos);
            constexpr std::string_view base_type_name{full_type_name.begin(), full_type_name.begin() + bracket_index};

            return base_type_name;
        }

        template <typename T>
        constexpr std::string_view serializable_type_name_for()
        {
            return Logger::details::type_name<T>();
        }

        // clang-format off
        template <typename Object>
        concept Serializable = requires(const Object& obj)
        {
            { do_serialize(std::declval<std::ostream&>(), obj) } -> std::same_as<bool>;
        };
        // clang-format on

        template <typename Object>
        struct SerializableObjectDescriptor
        {};

        template <Serializable T>
        bool serialize_internal(std::ostream&, const T&, std::size_t = 0) noexcept;

        template <typename T>
        bool serialize_internal(std::ostream&, const T&, std::size_t = 0) noexcept;

        template <Serializable Object>
        requires(has_target_v<Object>) bool serialize_with_target(
            std::ostream& os, const Object& object, std::size_t recursion_depth) noexcept
        {
            os << serializable_type_name_for<Object>() << "<> with ";
            return os.good() && do_serialize(os, object) && serialize_internal(os, object.target, recursion_depth + 1);
        }

        template <Serializable Object>
        bool serialize_with_target(std::ostream& os, const Object& object, std::size_t /*unused*/) noexcept
        {
            os << serializable_type_name_for<Object>() << " with ";
            return os.good() && do_serialize(os, object);
        }

        template <Serializable Object>
        bool serialize_internal(std::ostream& os, const Object& object, std::size_t recursion_depth) noexcept
        {
            for (std::size_t i{}; i != recursion_depth; ++i)
                os << "  ";
            return serialize_with_target(os, object, recursion_depth);
        }

        template <typename Object>
        bool serialize_internal(std::ostream& os, const Object& /*unused*/, std::size_t /*unused*/) noexcept
        {
            constexpr auto type_name = Logger::details::type_name<Object>();
            Logger::warn(
                "Object of type '",
                type_name,
                "' cannot be serialized. Make sure the function bool do_serialize(std::ostream&, const ",
                type_name,
                "&) exists and can be located by ADL\n");
            os << "__NONSERIALIZABLE__\n";
            return os.good();
        }
    } // namespace details

    template <typename Container>
    class SerializableObjectData
    {
    public:
        template <typename Contained>
        explicit SerializableObjectData(details::SerializableObjectDescriptor<Contained>&& /*unused*/)
            : serialize_{[](std::ostream& os, const Container& container) -> bool {
                  return details::serialize_internal(os, details::get_container_content<Contained>(container));
              }}
        {}

        bool serialize(const Container& container, std::ostream& os) const noexcept
        {
            return serialize_(os, container);
        }

    private:
        std::function<bool(std::ostream&, const Container&)> serialize_;
    };

} // namespace Raychel

#endif //!RAYCHEL_SERIALIZABLE_OBJECT_DATA_H
