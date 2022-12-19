include(FetchContent)

find_package(RaychelLogger QUIET)

if(NOT RaychelLogger_FOUND)

    message(STATUS "Could not find a local installation of RaychelLogger, using version off GitHub...")

    FetchContent_Declare(RAYCHEL_LOGGER
        GIT_REPOSITORY "https://github.com/Weckyy702/RaychelLogger"
        GIT_TAG "main"
    )

    FetchContent_MakeAvailable(RAYCHEL_LOGGER)

    set(RAYCHEL_LOGGER_EXTERNAL true)

endif()

find_package(RaychelCore QUIET)

if(NOT RaychelCore_FOUND)

    message(STATUS "Could not find a local installation of RaychelCore, using version off GitHub...")
    FetchContent_Declare(RAYCHEL_CORE
        GIT_REPOSITORY "https://github.com/Weckyy702/RaychelCore"
        GIT_TAG "main"
    )

    FetchContent_MakeAvailable(RAYCHEL_CORE)

    set(RAYCHEL_CORE_EXTERNAL true)

endif()

find_package(RaychelMath QUIET)

if(NOT RaychelMath_FOUND)

    message(STATUS "Could not find a local installation of RaychelMath, using version off GitHub...")
    FetchContent_Declare(RAYCHEL_MATH
        GIT_REPOSITORY "https://github.com/Weckyy702/RaychelMath"
        GIT_TAG "main"
    )

    FetchContent_MakeAvailable(RAYCHEL_MATH)

    set(RAYCHEL_MATH_EXTERNAL true)

endif()

if(RAYCHEL_BUILD_TESTS)

    find_package(Catch2 QUIET)

    if(NOT Catch2_FOUND)
        message(STATUS "Could not find a local installation of Catch2, using version off GitHub...")
    endif()

    FetchContent_Declare(CATCH_2
        GIT_REPOSITORY "https://github.com/catchorg/Catch2"
        GIT_TAG "v2.13.8"
    )

    FetchContent_MakeAvailable(CATCH_2)

    set(CATCH_2_EXTERNAL true)

endif()
