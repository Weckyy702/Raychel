if(NOT RAYCHEL_LOGGER_EXTERNAL)
    find_package(RaychelLogger REQUIRED)
endif()

if(NOT RAYCHEL_CORE_EXTERNAL)
    find_package(RaychelCore REQUIRED)
endif()

if(NOT RAYCHEL_MATH_EXTERNAL)
    find_package(RaychelMath REQUIRED)
endif()

file(GLOB RAYCHEL_TEST_SOURCES "*.test.cpp")

add_executable(Raychel_test
    ${RAYCHEL_TEST_SOURCES}
)

target_compile_features(Raychel_test PUBLIC cxx_std_20)

target_link_libraries(Raychel_test PUBLIC
    Raychel
)
