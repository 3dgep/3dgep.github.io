# Finds Emscripten, then loads its own CMake toolchain file.
#
# It looks in three places, in this order:
#   1. EMSDK       the emsdk folder, set when you activate emsdk (emsdk_env)
#   2. EMSCRIPTEN  the Emscripten folder itself
#   3. em-config   on the PATH, which every Emscripten install has
#                  (also Homebrew, apt and other package managers)

if(DEFINED ENV{EMSDK} AND EXISTS "$ENV{EMSDK}/upstream/emscripten")
    set(RAST_EMSCRIPTEN_ROOT "$ENV{EMSDK}/upstream/emscripten")
elseif(DEFINED ENV{EMSCRIPTEN} AND EXISTS "$ENV{EMSCRIPTEN}")
    set(RAST_EMSCRIPTEN_ROOT "$ENV{EMSCRIPTEN}")
else()
    find_program(RAST_EM_CONFIG NAMES em-config em-config.bat)
    if(RAST_EM_CONFIG)
        execute_process(
            COMMAND "${RAST_EM_CONFIG}" EMSCRIPTEN_ROOT
            OUTPUT_VARIABLE RAST_EMSCRIPTEN_ROOT
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE RAST_EM_CONFIG_RESULT
        )
        if(NOT RAST_EM_CONFIG_RESULT EQUAL 0)
            unset(RAST_EMSCRIPTEN_ROOT)
        endif()
    endif()
endif()

if(RAST_EMSCRIPTEN_ROOT)
    file(TO_CMAKE_PATH "${RAST_EMSCRIPTEN_ROOT}" RAST_EMSCRIPTEN_ROOT)
    set(RAST_EMSCRIPTEN_TOOLCHAIN "${RAST_EMSCRIPTEN_ROOT}/cmake/Modules/Platform/Emscripten.cmake")
endif()

if(NOT RAST_EMSCRIPTEN_TOOLCHAIN OR NOT EXISTS "${RAST_EMSCRIPTEN_TOOLCHAIN}")
    message(FATAL_ERROR
        "Emscripten was not found.\n"
        "Install the Emscripten SDK (https://emscripten.org/docs/getting_started/downloads.html) "
        "and activate it in this terminal first:\n"
        "  Windows:        emsdk_env.bat\n"
        "  Linux or macOS: source ./emsdk_env.sh\n"
        "Or install Emscripten with a package manager, so em-config is on the PATH.")
endif()

include("${RAST_EMSCRIPTEN_TOOLCHAIN}")
