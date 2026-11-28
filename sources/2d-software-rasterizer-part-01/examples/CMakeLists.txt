# One example per article. Each folder builds a single executable.
function(rast_add_example name)
    add_executable(${name} ${ARGN})
    target_link_libraries(${name} PRIVATE rast)
    rast_set_warnings(${name})

    if(EMSCRIPTEN)
        # Build a web page with a canvas, so the example runs in the browser:
        # bin/<name>/index.html, with index.js and index.wasm next to it.
        set(shell "${PROJECT_SOURCE_DIR}/cmake/web-shell.html")
        set_target_properties(${name} PROPERTIES
            OUTPUT_NAME index
            SUFFIX ".html"
            RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/${name}"
        )
        target_link_options(${name} PRIVATE "--shell-file=${shell}")
        set_property(TARGET ${name} APPEND PROPERTY LINK_DEPENDS "${shell}")
    endif()
endfunction()

add_subdirectory(part-01-framebuffer)
