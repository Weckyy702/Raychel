/**
 * \file SDFContainer.h
 * \author Weckyy702 (weckyy702@gmail.com)
 * \brief Header file for SDFContainer class
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
#ifndef RAYCHEL_SDF_CONTAINER_H
#define RAYCHEL_SDF_CONTAINER_H

#include "Raychel/Core/SDFPrimitives.h"
#include "Types.h"

#include "RaychelCore/Badge.h"
#include "RaychelCore/ClassMacros.h"

#include <RaychelCore/Raychel_assert.h>
#include <cstdint>
#include <memory>

namespace Raychel {

    template <typename T>
    constexpr bool has_custom_normal_v = requires(T t)
    {
        {
            evaluate_normal(t, vec3{})
            } -> std::same_as<vec3>;
    };

    namespace details {

        template <typename T>
        class TypeId
        {
            static char _;

        public:
            static std::uintptr_t id()
            {
                return reinterpret_cast<std::uintptr_t>(&_);
            }
        };

        template <typename T>
        char TypeId<T>::_{};

        struct ISDFContainerImpl
        {
            ISDFContainerImpl() = default;

            RAYCHEL_MAKE_NONCOPY_NONMOVE(ISDFContainerImpl)

            [[nodiscard]] virtual std::uintptr_t type_id() const noexcept = 0;

            virtual void debug_log() const noexcept = 0;

            virtual ~ISDFContainerImpl() = default;
        };

        template <typename T>
        class SDFContainerImpl final : public ISDFContainerImpl
        {
        public:
            explicit SDFContainerImpl(T&& object) noexcept(std::is_nothrow_move_constructible_v<T>)
                : object_{std::forward<T>(object)}
            {}

            void debug_log() const noexcept override
            {
                std::cout << "SDFContainer with object type " << Logger::details::type_name<T>() << " (type id " << type_id()
                          << ")";
                if constexpr (has_target_v<T>) {
                    std::cout << " and target ";
                    if constexpr (std::is_same_v<decltype(T::target), SDFContainer>) {
                        object_.target.unsafe_impl()->debug_log();
                    } else {
                        std::cout << Logger::details::type_name<decltype(T::target)>() << '\n';
                    }
                } else {
                    std::cout << '\n';
                }
            }

            [[nodiscard]] std::uintptr_t type_id() const noexcept override
            {
                return TypeId<T>::id();
            }

            [[nodiscard]] T& object() noexcept
            {
                return object_;
            }

            [[nodiscard]] const T& object() const noexcept
            {
                return object_;
            }

            ~SDFContainerImpl() override = default;

        private:
            T object_;
        };

        template <typename T>
        struct Eval
        {
            static double eval(ISDFContainerImpl* ptr, const vec3& p)
            {
                auto& obj = get_ref(ptr);
                return evaluate_sdf(obj, p);
            }

            static vec3 get_normal(ISDFContainerImpl* ptr, const vec3& p)
            {
                if constexpr (has_custom_normal_v<T>) {
                    auto& obj = get_ref(ptr);
                    return evaluate_normal(obj, p);
                }
                RAYCHEL_ASSERT_NOT_REACHED;
            }

            static T& get_ref(ISDFContainerImpl* ptr)
            {
                return reinterpret_cast<SDFContainerImpl<T>*>(ptr)->object();
            }
        };
    } // namespace details

    class SDFContainer
    {
        using EvalFunction = double (*)(details::ISDFContainerImpl*, const vec3&);
        using NormalFunction = vec3 (*)(details::ISDFContainerImpl*, const vec3&);

    public:
        template <typename T>
        using Impl = details::SDFContainerImpl<T>;

        template <typename T>
        requires(!std::is_same_v<std::remove_all_extents_t<T>, SDFContainer>) //otherwise the move constructor would be hidden
            explicit SDFContainer(T&& object) noexcept(std::is_nothrow_move_constructible_v<T>)
            : impl_{std::make_unique<Impl<T>>(std::forward<T>(object))},
              eval_{details::Eval<T>::eval},
              get_normal_(details::Eval<T>::get_normal),
              has_custom_normal_{has_custom_normal_v<T>}
        {}

        RAYCHEL_MAKE_NONCOPY(SDFContainer)

        RAYCHEL_MAKE_DEFAULT_MOVE(SDFContainer)

        [[nodiscard]] double evaluate(const vec3& p) const noexcept
        {
            return eval_(impl_.get(), p);
        }

        [[nodiscard]] bool has_custom_normal() const noexcept
        {
            return has_custom_normal_;
        }

        [[nodiscard]] vec3 get_normal(const vec3& p) const noexcept
        {
            return get_normal_(impl_.get(), p);
        }

        [[nodiscard]] auto type_id() const noexcept
        {
            return impl_->type_id();
        }

        [[nodiscard]] auto* unsafe_impl() const noexcept
        {
            return impl_.get();
        }

        ~SDFContainer() = default;

    private:
        std::unique_ptr<details::ISDFContainerImpl> impl_{};
        EvalFunction eval_;
        NormalFunction get_normal_;
        bool has_custom_normal_ : 1 {};
    };

    inline double evaluate_sdf(const SDFContainer& obj, const vec3& p)
    {
        return obj.evaluate(p);
    }
} // namespace Raychel

#endif //! RAYCHEL_SDF_CONTAINER_H
