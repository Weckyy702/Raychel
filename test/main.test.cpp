#include "Raychel/Core/Deserialize.h"
#include "Raychel/Core/Raymarch.h"
#include "Raychel/Core/SDFBooleans.h"
#include "Raychel/Core/SDFModifiers.h"
#include "Raychel/Core/SDFPrimitives.h"
#include "Raychel/Core/SDFTransforms.h"
#include "Raychel/Core/Scene.h"
#include "Raychel/Core/Types.h"
#include "Raychel/Render/RenderUtils.h"
#include "Raychel/Render/Renderer.h"

#include "RaychelMath/vector.h"

#include <RaychelMath/Quaternion.h>
#include <RaychelMath/color.h>
#include <RaychelMath/constants.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

struct FlatMaterial
{
    Raychel::color surface_color{};
};

struct ReflectiveMaterial
{
    Raychel::color reflectivity{};
};

struct DiffuseMaterial
{
    Raychel::color surface_color{};
};

struct TransparentMaterial
{
    Raychel::color transparency{};
    double ior{1.0};
    double ior_variation{.1};
};

struct DebugMaterial
{};

template <>
struct Raychel::is_transparent_material<TransparentMaterial> : std::true_type
{};

Raychel::color get_surface_color(const FlatMaterial& material, const Raychel::ShadingData& /*unused*/) noexcept
{
    return material.surface_color;
}

Raychel::color get_surface_color(const ReflectiveMaterial& material, const Raychel::ShadingData& data) noexcept
{
    return Raychel::get_shaded_color(Raychel::RenderData{
               .origin = data.position,
               .direction = reflect(data.incoming_direction, data.normal),
               .state = data.state,
               .recursion_depth = data.recursion_depth}) *
           material.reflectivity;
}

Raychel::color get_surface_color(const DiffuseMaterial& material, const Raychel::ShadingData& data) noexcept
{
    return Raychel::get_diffuse_lighting(data) * material.surface_color;
}

Raychel::color get_surface_color(const TransparentMaterial& material, const Raychel::ShadingData& data) noexcept
{
    return Raychel::get_refraction(Raychel::RefractionData{
               .surface_point = data.position,
               .incoming_direction = data.incoming_direction,
               .normal = data.normal,
               .material_ior = material.ior,
               .ior_variation = material.ior_variation,
               .state = data.state,
               .recursion_depth = data.recursion_depth}) *
           material.transparency;
}

Raychel::color get_surface_color(const DebugMaterial& /*unused*/, const Raychel::ShadingData& data) noexcept
{
    const auto [x, y, z] = data.normal;
    return Raychel::color{std::abs(x), std::abs(y), std::abs(z)};
}

double get_material_ior(const TransparentMaterial& material) noexcept
{
    return material.ior;
}

[[maybe_unused]] static void write_framebuffer(const std::string& file_name, const Raychel::Framebuffer& framebuffer) noexcept
{
    //Don't bother writing an empty framebuffer
    if (framebuffer.pixel_data.empty()) {
        return;
    }

    const auto label = Logger::startTimer("Write time");
    std::ofstream output_image{file_name};
    if (!output_image) {
        Logger::error("Unable to open output file '", file_name, "'\n");
        return;
    }

    output_image << "P6\n" << framebuffer.size.x() << ' ' << framebuffer.size.y() << '\n' << "255\n";
    for (const auto& pixel : framebuffer.pixel_data) {
        const auto pixel_rgb = convert_color<std::uint8_t>(pixel);
        if (!output_image.write(reinterpret_cast<const char*>(&pixel_rgb), sizeof(pixel_rgb)).good()) {
            Logger::error("Error while writing output file!\n");
            break;
        }
    }
    Logger::logDuration(label);
}

[[maybe_unused]] static Raychel::Framebuffer fat_pixels_to_regular(const Raychel::FatFramebuffer& input) noexcept
{
    std::vector<Raychel::color> output{};
    output.reserve(input.size.x() * input.size.y());
    for (const auto& pixel : input) {
        output.emplace_back(pixel.noisy_color);
    }
    return {input.size, std::move(output)};
}

[[maybe_unused]] static void build_cornell_box(Raychel::Scene& scene)
{
    using namespace Raychel;

    constexpr auto room_size = 1.0;
    constexpr auto box_size = room_size * 1.1;
    constexpr auto slim = 0.1;

    //Floor
    scene.add_object(Translate{Box{vec3{box_size, slim, box_size}}, vec3{0, -room_size, 0}}, DiffuseMaterial{color{1, 1, 1}});
    //Ceiling
    scene.add_object(Translate{Box{vec3{box_size, slim, box_size}}, vec3{0, room_size, 0}}, FlatMaterial{color{1, 1, .9} * 2.5});

    //Left wall
    scene.add_object(
        Translate{Box{vec3{slim, box_size, box_size}}, vec3{-room_size * 1.01, 0, 0}}, DiffuseMaterial{color{1, 0, 0}});
    //Right wall
    scene.add_object(Translate{Box{vec3{slim, box_size, box_size}}, vec3{room_size, 0, 0}}, DiffuseMaterial{color{0, 1, 0}});
    //Back wall
    scene.add_object(Translate{Box{vec3{box_size, box_size, slim}}, vec3{0, 0, room_size}}, DiffuseMaterial{color{1, 1, 1}});

    //Back sphere
    scene.add_object(
        Translate{Sphere{0.5}, vec3{-room_size + slim + 0.5, -room_size + 1.1 * slim + 0.5, room_size - 2 * slim - 0.5}},
        ReflectiveMaterial{color_from_hex<double>(0xFF5733) * 0.95});

    //Front box
    scene.add_object(
        Translate{
            Rotate{Sphere{.25}, rotate_around(vec3{0, 1, 0}, 60 * deg_to_rad<double>)},
            vec3{room_size - slim - .5625, -room_size + slim + .25, -room_size + .375}},
        TransparentMaterial{.transparency = color_from_hex<double>(0xa8ccd7), .ior = 1.5});
}

bool do_serialize(std::ostream& os, const FlatMaterial& material) noexcept
{
    os << material.surface_color << '\n';
    return os.good();
}

bool do_serialize(std::ostream& os, const ReflectiveMaterial& material) noexcept
{
    os << material.reflectivity << '\n';
    return os.good();
}

bool do_serialize(std::ostream& os, const DiffuseMaterial& material) noexcept
{
    os << material.surface_color << '\n';
    return os.good();
}

std::optional<DiffuseMaterial> do_deserialize(std::istream& is, Raychel::DeserializationTag<DiffuseMaterial>) noexcept
{
    Raychel::color c{};
    if (!(is >> c))
        return std::nullopt;

    return DiffuseMaterial{c};
}

std::optional<FlatMaterial> do_deserialize(std::istream& is, Raychel::DeserializationTag<FlatMaterial>) noexcept
{
    Raychel::color c{};
    if (!(is >> c))
        return std::nullopt;

    return FlatMaterial{c};
}

std::optional<ReflectiveMaterial> do_deserialize(std::istream& is, Raychel::DeserializationTag<ReflectiveMaterial>) noexcept
{
    Raychel::color c{};
    if (!(is >> c))
        return std::nullopt;

    return ReflectiveMaterial{c};
}

template <Raychel::Arithmetic T, std::convertible_to<T> T_>
constexpr Raychel::basic_color<T> lerp(const Raychel::basic_color<T>& a, const Raychel::basic_color<T>& b, T_ x)
{
    return (b * x) + (a * (1.0 - x));
}

int main()
{
    Logger::setMinimumLogLevel(Logger::LogLevel::debug);

    using namespace Raychel;

#if 0
    Scene scene;
    build_cornell_box(scene);

    std::ofstream out_stream{"cornell_box.txt"};

    serialize_scene(scene, out_stream);
#endif
#if 0
    std::ifstream in_file{"cornell_box.txt"};

    auto scene = deserialize_scene(
        in_file,
        object_deserializers<Translate<>, Rotate<>, Sphere, Box>(),
        material_deserializers<DiffuseMaterial, FlatMaterial, ReflectiveMaterial>());

#endif

#if 1
    Scene scene;
    //scene.add_object(Translate{Sphere{.75}, vec3{0, -.25, 0}}, TransparentMaterial{color{1, .65, .45}, 1.5, .05});
    //scene.add_object(Translate{Plane{vec3{0, 1, 0}}, vec3{0, -1.01, 0}}, DiffuseMaterial{color{1}});
    //scene.add_object(Translate{Box{vec3{.1, 2, 2}}, vec3{-2, 0, 0}}, FlatMaterial{color{10}});

    scene.add_object(Translate{Sphere{.5}, vec3{-3.5, 2.5, -1.5}}, FlatMaterial{(color_from_hex<double>(0x2FE3E0)) * 10});
    scene.add_object(Translate{Sphere{.5}, vec3{2.5, 2.5, 1.5}}, FlatMaterial{color_from_hex<double>(0xD01C1F) * 10});
    scene.add_object(Translate{Sphere{.05}, vec3{0, .8, 0}}, FlatMaterial{color{1, .75, .5625} * 5});

    scene.add_object(
        Difference{Translate{Sphere{.5}, vec3{0, .85, 0}}, Rounded{Box{vec3{1, 1, 1}}, 0.1}}, DiffuseMaterial{color{1, 1, 1}});
    scene.set_background_function([](const RenderData& data) {
/*        if (data.direction.z() < -0.99) {
            return color_from_hex<double>(0xfdd835) * 15;
        }
        return lerp(
                   color_from_hex<double>(0xFFC922),
                   color_from_hex<double>(0x87CEEB),
                   std::pow(std::abs(data.direction.y()) + 0.2, 1.0 / 3.0)) *
               0.75;*/
        (void)data;
        return color{};
    });
#endif

    for (const auto& obj : scene.objects()) {
        obj.unsafe_impl()->debug_log();
    }

    const auto rendered_image = render_scene(
        scene,
        Camera{
            .transform = {.offset = vec3{0, 2.5, -2.5}, .rotation = rotate_around(vec3{1, 0, 0}, quarter_pi<double>)},
            .zoom = 1.0},
        RenderOptions{
            .output_size = Size2D{1920, 1080} / 2U,
            .max_ray_steps = 4096,
            .max_recursion_depth = 100,
            .samples_per_pixel = 1U << 10U,
        });

    write_framebuffer("out.ppm", fat_pixels_to_regular(rendered_image));
    return 0;
}
