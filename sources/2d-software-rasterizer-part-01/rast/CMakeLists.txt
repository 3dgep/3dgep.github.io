add_library(rast STATIC
    include/rast/Color.hpp
    include/rast/Surface.hpp
    include/rast/Window.hpp
    src/Surface.cpp
    src/Window.cpp
)

target_include_directories(rast PUBLIC include)

# SDL3 is an implementation detail: only Window.cpp includes it.
target_link_libraries(rast PRIVATE SDL3::SDL3)

rast_set_warnings(rast)

if(EMSCRIPTEN)
    # Window::Present yields to the browser with emscripten_sleep,
    # so every program that links rast needs Asyncify.
    target_link_options(rast PUBLIC -sASYNCIFY)
endif()
