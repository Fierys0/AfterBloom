# ============================================================
# Game-level Android CMakeLists (packaging via raymob/Gradle)
# ------------------------------------------------------------
# This file is the game project's Android build entry point.
# The fumbo ENGINE handles raymob/raylib inclusion internally.
# This file only compiles GAME sources and links fumbo_engine.
# ============================================================

cmake_minimum_required(VERSION 3.22.1)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

project("${APP_LIB_NAME}")

# ===== Paths =====
# This file lives at: android/app/src/main/cpp/
# Project root (AtherReboot/) is 5 levels up.
get_filename_component(ATHER_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../../../.." ABSOLUTE)
set(ENGINE_DIR "${ATHER_ROOT}/engine")
set(SRC_DIR    "${ATHER_ROOT}/src")

# ===== Fumbo Engine (handles raylib + raymob internally on Android) =====
add_subdirectory(${ENGINE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/fumbo_engine_build)

# ===== Generate cmake/generated_buildinfo.hpp at configure time =====
# This mirrors what cmake/buildinfo.cmake does for desktop builds.
string(TIMESTAMP BUILD_TIME "%y%m%d-%H%M")
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/cmake/generated_buildinfo.hpp"
    "#pragma once\n"
    "#define BUILD_TIME \"${BUILD_TIME}\"\n"
)

# ===== Game sources (game project's responsibility, not the engine's) =====
file(GLOB_RECURSE GAME_SOURCES "${SRC_DIR}/game/*.cpp")

# ===== Unified entry point (contains android_main via #ifdef PLATFORM_ANDROID) =====
list(APPEND GAME_SOURCES "${SRC_DIR}/main.cpp")

# ===== android_native_app_glue =====
# Included here so the linker can find ANativeActivity_onCreate (required by raymob)
include_directories(${ANDROID_NDK}/sources/android/native_app_glue/)
list(APPEND GAME_SOURCES ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c)

# ===== Shared library (single .so output for Android) =====
add_library(${APP_LIB_NAME} SHARED ${GAME_SOURCES})

target_compile_definitions(${APP_LIB_NAME} PRIVATE PLATFORM_ANDROID)

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    target_compile_definitions(${APP_LIB_NAME} PRIVATE _DEBUG DEBUG)
endif()

target_include_directories(${APP_LIB_NAME} PRIVATE
    ${ENGINE_DIR}                    # fumbo.hpp
    ${SRC_DIR}                       # game headers
    ${CMAKE_CURRENT_BINARY_DIR}      # cmake/generated_buildinfo.hpp
)

# ===== Link against fumbo_engine (engine handles raylib/raymob/etc.) =====
target_link_libraries(${APP_LIB_NAME}
    fumbo_engine
    log
    android
    EGL
    GLESv2
    OpenSLES
)
