cmake_minimum_required(VERSION 3.14)

project(Raychel LANGUAGES CXX)

include(cmake/find_dependencies.cmake)

set(RAYCHEL_INCLUDE_DIR
    "include/Raychel"
)

add_library(Raychel SHARED
    "${RAYCHEL_INCLUDE_DIR}/Core/Types.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/SDFContainer.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/Raymarch.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/ZigguratNormal.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/Scene.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/Serialize.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/Deserialize.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/SDFPrimitives.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/SDFTransforms.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/SDFBooleans.h"
    "${RAYCHEL_INCLUDE_DIR}/Core/SDFModifiers.h"

    "${RAYCHEL_INCLUDE_DIR}/Render/MaterialContainer.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/Framebuffer.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/Renderer.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/RenderUtils.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/Camera.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/RayHistogram.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/FatPixel.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/Denoise.h"
    "${RAYCHEL_INCLUDE_DIR}/Render/Materials.h"

    "src/Core/Scene.cpp"
    "src/Core/ZigguratNormal.cpp"
    "src/Render/Renderer.cpp"
    "src/Render/RenderUtils.cpp"
    "src/Render/Denoise.cpp"
    "src/Core/Serialize.cpp"
    "src/Core/Deserialize.cpp"
    "src/Core/SDFPrimitives.cpp"
    "src/Core/Raymarch.cpp"
)

target_include_directories(Raychel PUBLIC
    "include"
)

target_compile_features(Raychel PUBLIC cxx_std_20)
target_compile_options(Raychel PUBLIC
    -Wall
    -Wextra
    -Wshadow
    -Wpedantic
    -Wconversion
    -Werror
)

target_link_libraries(Raychel PUBLIC
    RaychelLogger
    RaychelCore
    RaychelMath
    tbb
)

add_subdirectory(test)
