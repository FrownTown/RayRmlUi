cmake_minimum_required(VERSION 3.20...4.99)
project(rayRmlUi VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# ── raylib ───────────────────────────────────────────────────────────────────
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        5.5
    GIT_SHALLOW    TRUE
    PATCH_COMMAND  ${CMAKE_COMMAND} -DSRC=<SOURCE_DIR>/src/rcore.c -P ${CMAKE_SOURCE_DIR}/cmake/patch_rcore.cmake
)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(OPENGL_VERSION "4.3" CACHE STRING "" FORCE)
FetchContent_MakeAvailable(raylib)

# ── FreeType (required by RmlUi for TTF rendering) ───────────────────────────
FetchContent_Declare(
    freetype
    GIT_REPOSITORY https://github.com/freetype/freetype.git
    GIT_TAG        VER-2-13-3
    GIT_SHALLOW    TRUE
)
set(FT_DISABLE_ZLIB     ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2    ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG      ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BROTLI   ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(freetype)
if(NOT TARGET Freetype::Freetype)
    add_library(Freetype::Freetype ALIAS freetype)
endif()

# ── RmlUi ────────────────────────────────────────────────────────────────────
FetchContent_Declare(
    rmlui
    GIT_REPOSITORY https://github.com/mikke89/RmlUi.git
    GIT_TAG        6.2
    GIT_SHALLOW    TRUE
)
set(RMLUI_SAMPLES        OFF CACHE BOOL "" FORCE)
set(RMLUI_LUA_BINDINGS   OFF CACHE BOOL "" FORCE)
set(RMLUI_SVG_PLUGIN     OFF CACHE BOOL "" FORCE)
set(RMLUI_LOTTIE_PLUGIN  OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(rmlui)

# ── NanoSVG (header-only; used by RmlRendererRaylib for .svg textures) ───────
FetchContent_Declare(
    nanosvg
    GIT_REPOSITORY https://github.com/memononen/nanosvg.git
    GIT_TAG        master
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(nanosvg)

# ── rayRmlUi library ─────────────────────────────────────────────────────────
add_library(rayRmlUi STATIC
    src/rayRmlUi.cpp
    src/RmlRendererRaylib.cpp
    src/RmlSystemRaylib.cpp
    src/RmlInputRaylib.cpp
)

target_include_directories(rayRmlUi PUBLIC
    src/
)

target_include_directories(rayRmlUi PRIVATE
    ${nanosvg_SOURCE_DIR}/src
    ${raylib_SOURCE_DIR}/src          # for external/glad.h
)

target_link_libraries(rayRmlUi PUBLIC
    raylib
    rmlui_core
)

# ── Installation ──────────────────────────────────────────────────────────────
install(TARGETS rayRmlUi ARCHIVE DESTINATION lib)
install(FILES src/rayRmlUi.h DESTINATION include)
