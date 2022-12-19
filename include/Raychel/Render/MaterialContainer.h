/**
* \file MaterialContainer.h
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Header file for MaterialContainer class
* \date 2022-04-10
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
#ifndef RAYCHEL_MATERIAL_CONTAINER_H
#define RAYCHEL_MATERIAL_CONTAINER_H

#include "Materials.h"

#include "Raychel/Core/Types.h"
#include "RaychelCore/Badge.h"
#include "RaychelCore/ClassMacros.h"

#include <memory>

namespace Raychel {

    namespace details {

        struct IMaterialContainerImpl
        {
            IMaterialContainerImpl() = default;

            RAYCHEL_MAKE_NONCOPY_NONMOVE(IMaterialContainerImpl)

            [[nodiscard]] virtual color get_surface_color_internal(const ShadingData& data) const noexcept = 0;

            [[nodiscard]] virtual double get_material_ior_internal() const noexcept = 0;

            virtual ~IMaterialContainerImpl() = default;
        };

        template <typename T>
        class MaterialContainerImpl final : public IMaterialContainerImpl
        {
        public:
            explicit MaterialContainerImpl(T&& object) noexcept(std::is_nothrow_move_constructible_v<T>)
                : object_{std::forward<T>(object)}
            {}

            [[nodiscard]] color get_surface_color_internal(const ShadingData& data) const noexcept override
            {
                return get_surface_color(object_, data);
            }

            [[nodiscard]] double get_material_ior_internal() const noexcept override
            {
                if constexpr (is_transparent_material_v<T>)
                    return get_material_ior(object_);
                return 1.0;
            }

            [[nodiscard]] T& object() noexcept
            {
                return object_;
            }

            [[nodiscard]] const T& object() const noexcept
            {
                return object_;
            }

        private:
            T object_;
        };

    } // namespace details

    class MaterialContainer
    {
    public:
        template <typename T>
        using Impl = details::MaterialContainerImpl<T>;

        template <typename T>
        requires(!std::is_same_v<T, MaterialContainer>) explicit MaterialContainer(T&& object) noexcept(
            std::is_nothrow_move_constructible_v<T>)
            : impl_{std::make_unique<details::MaterialContainerImpl<T>>(std::forward<T>(object))}
        {}

        RAYCHEL_MAKE_NONCOPY(MaterialContainer)
        RAYCHEL_MAKE_DEFAULT_MOVE(MaterialContainer)

        [[nodiscard]] color get_surface_color(const ShadingData& data) const noexcept
        {
            return impl_->get_surface_color_internal(data);
        }

        [[nodiscard]] double get_material_ior() const noexcept
        {
            return impl_->get_material_ior_internal();
        }

        [[nodiscard]] auto* unsafe_impl() const noexcept
        {
            return impl_.get();
        }

        ~MaterialContainer() = default;

    private:
        std::unique_ptr<details::IMaterialContainerImpl> impl_{};
    };

} //namespace Raychel

#endif //!RAYCHEL_MATERIAL_CONTAINER_H
