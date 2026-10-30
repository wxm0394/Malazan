cmake_minimum_required(VERSION 3.20)
project(MalazanEngine VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL3 REQUIRED sdl3)

include_directories(include ${SDL3_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(darujhistan_slice ${SOURCES})
target_link_libraries(darujhistan_slice PRIVATE ${SDL3_LIBRARIES})

add_custom_command(TARGET darujhistan_slice POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:darujhistan_slice>/assets
)
