cmake_minimum_required(VERSION 3.28)

project(2d-software-rasterizer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Put every executable next to each other, so they are easy to find.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)

# SDL3 opens the window and shows our framebuffer on screen.
# We build it as a static library, so there is no DLL to copy around.
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    SDL3
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG        release-3.4.20
    GIT_SHALLOW    TRUE
    SYSTEM
)
FetchContent_MakeAvailable(SDL3)

# stb_image loads image files (PNG, JPEG, BMP, TGA, ...) into memory.
# stb has no CMake project and no releases, so we download a fixed commit
# and make an interface library for its folder of headers.
FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/6b5799bae8a1fe3792691ef4b72ae26434c3e799.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(stb)

add_library(stb INTERFACE)
# SYSTEM: the compiler doesn't warn about code in these headers, because it isn't ours.
target_include_directories(stb SYSTEM INTERFACE ${stb_SOURCE_DIR})

# Warnings are errors waiting to happen, so we ask for all of them.
function(rast_set_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive- /utf-8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
endfunction()

add_subdirectory(rast)
add_subdirectory(examples)
