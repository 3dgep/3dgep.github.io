# One example per article. Each folder builds a single executable:
#
#   rast_add_example(<name> <sources>... [ASSETS <folder>])
#
# ASSETS is a folder of files that the example loads at run time, like images.
# The program finds them in a folder called "assets", next to the executable
# (or in the browser's virtual file system, on the web).
function(rast_add_example name)
    cmake_parse_arguments(PARSE_ARGV 1 arg "" "ASSETS" "")
    add_executable(${name} ${arg_UNPARSED_ARGUMENTS})
    target_link_libraries(${name} PRIVATE rast)
    rast_set_warnings(${name})

    if(arg_ASSETS)
        set(assets "${CMAKE_CURRENT_SOURCE_DIR}/${arg_ASSETS}")
        file(GLOB_RECURSE asset_files CONFIGURE_DEPENDS "${assets}/*")
    endif()

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

        if(arg_ASSETS)
            # Pack the assets into index.data, which the page loads into the virtual
            # file system before main starts. "@assets" is their folder in there.
            target_link_options(${name} PRIVATE "--preload-file=${assets}@assets")
            set_property(TARGET ${name} APPEND PROPERTY LINK_DEPENDS ${asset_files})
        endif()
    elseif(arg_ASSETS)
        # Copy the assets next to the executable, whenever they change.
        add_custom_target(${name}-assets
            COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different "${assets}" "$<TARGET_FILE_DIR:${name}>/assets"
            DEPENDS ${asset_files}
            COMMENT "Copying the assets of ${name}"
        )
        add_dependencies(${name} ${name}-assets)

        # The program looks for the assets in the current working directory,
        # so Visual Studio runs it in the folder of the executable.
        set_target_properties(${name} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${name}>")
    endif()
endfunction()

add_subdirectory(part-01-framebuffer)
add_subdirectory(part-02-clipping)
add_subdirectory(part-03-lines)
add_subdirectory(part-04-circles)
add_subdirectory(part-05-triangles)
add_subdirectory(part-06-barycentric)
add_subdirectory(part-07-texturing)
