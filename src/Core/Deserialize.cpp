/**
* \file Deserialize.cpp
* \author Weckyy702 (weckyy702@gmail.com)
* \brief Implementation file for Deserialize class
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

#include "Raychel/Core/Deserialize.h"
#include "Raychel/Core/SDFPrimitives.h"

namespace Raychel {

    struct DeserializerState
    {
        std::istream& input_stream;
        const std::vector<details::DeserializerPtr<SDFContainer>>& object_deserializers;
        const std::vector<details::DeserializerPtr<MaterialContainer>>& material_deserializers;

        std::vector<SDFContainer> objects{};
        std::vector<SerializableObjectData<SDFContainer>> object_serializers{};

        std::vector<MaterialContainer> materials{};
        std::vector<SerializableObjectData<MaterialContainer>> material_serializers{};

        bool is_in_object_section{true};
    };

    template <typename Container>
    struct ContainerAndSerializer
    {
        Container container;
        SerializableObjectData<Container> serializer;
    };

    template <typename T>
    ContainerAndSerializer(T, SerializableObjectData<T>) -> ContainerAndSerializer<T>;

    enum class ParseObjectFromLineResult {
        ok = 0,
        entered_material_section,
        no_type_name_separator,
        empty_line,
    };

    static bool check_object_header(std::istream& is) noexcept
    {
        std::string line{};
        std::getline(is, line);

        if (line == "--BEGIN SURFACES--")
            return true;

        Logger::warn("Incorrect surface section header '", line, "'\n");
        return false;
    }

    static std::string get_shortened_line(std::istream& is) noexcept
    {
        std::string line{};
        if (!std::getline(is, line))
            return {};

        //skip whitespace characters
        auto current_char = line.begin();
        while (std::isspace(*current_char) != 0) {
            current_char++;
        }

        return {current_char, line.end()};
    }

    template <typename Container>
    static details::IDeserializer<Container>* find_deserializer_for(
        std::string_view type_name, const std::vector<details::DeserializerPtr<Container>>& deserializers) noexcept
    {
        for (const auto& deserializer : deserializers) {
            if (deserializer->contained_type_name() == type_name) {
                return deserializer.get();
            }
        }
        Logger::warn("Could not find deserializer for type name '", type_name, "'\n");
        return nullptr;
    }

    template <typename Container>
    static std::optional<ContainerAndSerializer<Container>> parse_object(
        std::string_view type_name, std::string_view rest_of_line,
        const std::vector<details::DeserializerPtr<Container>>& deserializers) noexcept
    {
        const auto maybe_deserializer = find_deserializer_for(type_name, deserializers);
        if (maybe_deserializer == nullptr)
            return std::nullopt;

        std::stringstream line_stream{std::string{rest_of_line}}; //Hmmmmmmmmmmmmmmmmmmm
        auto maybe_object = maybe_deserializer->deserialize(line_stream, std::nullopt);
        if (!maybe_object.has_value()) {
            Logger::warn("Could not deserialize object of type '", type_name, " with data '", rest_of_line, "'\n");
            return std::nullopt;
        }
        return ContainerAndSerializer{std::move(maybe_object).value(), maybe_deserializer->get_serializer()};
    }

    template <typename Container>
    static std::optional<ContainerAndSerializer<Container>> parse_targeted(
        DeserializerState& state, std::string_view type_name, std::string_view rest_of_line,
        const std::vector<details::DeserializerPtr<Container>>& deserializers) noexcept
    {
        const auto maybe_deserializer = find_deserializer_for(type_name, deserializers);
        if (maybe_deserializer == nullptr)
            return std::nullopt;

        auto [maybe_target_and_serializer, result] = parse_object_from_line(state, deserializers);
        if (result != ParseObjectFromLineResult::ok)
            return std::nullopt;
        if (!maybe_target_and_serializer.has_value())
            return std::nullopt;

        Container target = std::move(maybe_target_and_serializer).value().container;

        std::stringstream line_stream{std::string{rest_of_line}};
        auto maybe_object = maybe_deserializer->deserialize(line_stream, std::move(target));
        if (!maybe_object.has_value()) {
            Logger::warn("Could not deserialize object of type '", type_name, " with data '", rest_of_line, "'\n");
            return std::nullopt;
        }

        return ContainerAndSerializer{std::move(maybe_object).value(), maybe_deserializer->get_serializer()};
    }

    template <typename Container>
    static std::optional<ContainerAndSerializer<Container>> parse_type(
        DeserializerState& state, std::string_view type_name, std::string_view rest_of_line,
        const std::vector<details::DeserializerPtr<Container>>& deserializers) noexcept
    {
        //If the type is has a target, recursively resolve that target
        if (type_name.ends_with("<>")) {
            type_name.remove_suffix(2U);
            return parse_targeted(state, type_name, rest_of_line, deserializers);
        }
        return parse_object(type_name, rest_of_line, deserializers);
    }

    template <typename Container>
    static bool add_object(
        std::optional<ContainerAndSerializer<Container>> maybe_object, std::vector<Container>& containers,
        std::vector<SerializableObjectData<Container>>& serializers) noexcept
    {
        if (!maybe_object.has_value())
            return false;
        auto [container, serializer] = std::move(maybe_object).value();
        containers.emplace_back(std::move(container));
        serializers.emplace_back(std::move(serializer));
        return true;
    }

    template <typename Container>
    static std::pair<std::optional<ContainerAndSerializer<Container>>, ParseObjectFromLineResult> parse_object_from_line(
        DeserializerState& state, const std::vector<details::DeserializerPtr<Container>>& deserializers) noexcept
    {
        const auto line = get_shortened_line(state.input_stream);

        if (line.empty())
            return {std::nullopt, ParseObjectFromLineResult::empty_line};

        if (line == "--BEGIN MATERIALS--") {
            return {std::nullopt, ParseObjectFromLineResult::entered_material_section};
        }

        const auto end_of_type_name_index = line.find(" with ");
        if (end_of_type_name_index == std::string::npos) {
            Logger::warn("Incorrect type name separator!\n");
            return {std::nullopt, ParseObjectFromLineResult::no_type_name_separator};
        }
        const auto end_of_type_name = line.begin() + static_cast<std::ptrdiff_t>(end_of_type_name_index);
        std::string_view type_name{line.begin(), end_of_type_name};
        std::string_view rest_of_line{end_of_type_name + 6, line.end()};

        return std::make_pair(parse_type(state, type_name, rest_of_line, deserializers), ParseObjectFromLineResult::ok);
    }

    static bool parse_line(DeserializerState& state) noexcept
    {
        if (state.is_in_object_section) {
            auto [maybe_object, result] = parse_object_from_line(state, state.object_deserializers);
            if (result == ParseObjectFromLineResult::entered_material_section) {
                state.is_in_object_section = false;
                return true;
            }
            if (result == ParseObjectFromLineResult::empty_line)
                return true;
            if (result != ParseObjectFromLineResult::ok)
                return false;
            return add_object(std::move(maybe_object), state.objects, state.object_serializers);
        }
        auto [maybe_material, result] = parse_object_from_line(state, state.material_deserializers);
        if (result == ParseObjectFromLineResult::entered_material_section) {
            Logger::warn("Entered material section twice!\n");
            return false;
        }
        if (result == ParseObjectFromLineResult::empty_line)
            return true;
        if (result != ParseObjectFromLineResult::ok) {
            return false;
        }
        return add_object(std::move(maybe_material), state.materials, state.material_serializers);
    }

    static void place_dummy(DeserializerState& state) noexcept
    {
        if (state.is_in_object_section) {
            state.objects.emplace_back(DeserializationErrorPlaceHolder{});
            state.object_serializers.emplace_back(details::SerializableObjectDescriptor<DeserializationErrorPlaceHolder>{});
            return;
        }
        state.materials.emplace_back(DeserializationErrorMaterial{});
        state.material_serializers.emplace_back((details::SerializableObjectDescriptor<DeserializationErrorMaterial>{}));
    }

    Scene deserialize_scene(
        std::istream& is, const std::vector<details::DeserializerPtr<SDFContainer>>& object_deserializers,
        const std::vector<details::DeserializerPtr<MaterialContainer>>& material_deserializers) noexcept
    {
        DeserializerState state{is, object_deserializers, material_deserializers};

        if (!check_object_header(is))
            return {};

        while (state.input_stream) {
            if (!parse_line(state)) {
                Logger::debug("Placing dummy\n");
                place_dummy(state);
            }
        }

        if (state.is_in_object_section) {
            Logger::warn("Parser did not leave object section! Incorrect material section header?\n");
        }

        return Scene::unsafe_from_data(
            std::move(state.objects),
            std::move(state.object_serializers),
            std::move(state.materials),
            std::move(state.material_serializers));
    }

} //namespace Raychel
