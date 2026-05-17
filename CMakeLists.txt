cmake_minimum_required(VERSION 3.16)

project(AfterBloom VERSION 0.1 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(default_build_type "Release")

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  message(STATUS "Setting build type to '${default_build_type}' as none was specified.")
  set(CMAKE_BUILD_TYPE "${default_build_type}" CACHE STRING
      "Choose the type of build." FORCE)

  # Set the possible values for cmake-gui drop-down menus
  set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
    "Debug" "Release" "MinSizeRel" "RelWithDebInfo")
endif()

# Build info generation (per build)
add_custom_target(
    generate_buildinfo ALL
    COMMAND ${CMAKE_COMMAND}
            -P ${CMAKE_SOURCE_DIR}/cmake/buildinfo.cmake
)

# Enable Position Independent Code for all targets (required for shared libraries)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Raylib - build as static library to combine with fumbo
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build Raylib as a static library" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
add_subdirectory(engine/lib/raylib)
# Set FUMBO_WITH_VIDEO to OFF in the project CMake
set(FUMBO_WITH_VIDEO OFF CACHE BOOL "Disable video support" FORCE)
add_subdirectory(engine)

# Set fumbo_engine library output directory
set_target_properties(fumbo_engine PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/afterbloom
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/afterbloom
)

file(GLOB_RECURSE GAME_SOURCES src/game/*.cpp src/main.cpp)

add_executable(${PROJECT_NAME} ${GAME_SOURCES})

# Put executable in "afterbloom" folder
set_target_properties(${PROJECT_NAME} PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/afterbloom
)

# Set RPATH so executable can find fumbo shared library
if(UNIX AND NOT APPLE)
  set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "$ORIGIN"
        INSTALL_RPATH "$ORIGIN"
    )
endif()

# Make sure buildinfo is generated before compiling
add_dependencies(${PROJECT_NAME} generate_buildinfo)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        src
        ${CMAKE_BINARY_DIR}   # for generated_buildinfo.hpp
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        fumbo_engine
)

if(WIN32)
  # Copy mpv DLL next to executable
  # We still need to know where it is.
  set(MPV_ROOT ${CMAKE_SOURCE_DIR}/engine/lib/windows/mpv)

  target_include_directories(${PROJECT_NAME}
        PRIVATE
            ${MPV_ROOT}/include
    )

  add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            ${MPV_ROOT}/libmpv-2.dll
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# ===== Asset Packing Tool =====
add_executable(pack_assets engine/tools/pack_assets.cpp)
target_include_directories(pack_assets PRIVATE engine)
target_compile_features(pack_assets PRIVATE cxx_std_17)

# Pack assets into encrypted .fpk files by type
# Images pack
add_custom_target(pack_images
    COMMAND pack_assets ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/images.fpk png,jpg,jpeg,bmp,svg
    DEPENDS pack_assets
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing image assets..."
)

# Fonts pack
add_custom_target(pack_fonts
    COMMAND pack_assets ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/fonts.fpk ttf,otf
    DEPENDS pack_assets
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing font assets..."
)

# Audio pack
add_custom_target(pack_audio
    COMMAND pack_assets ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/audio.fpk ogg,mp3,wav,flac
    DEPENDS pack_assets
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Packing audio assets..."
)

# Make sure main executable depends on all packed assets
add_dependencies(${PROJECT_NAME} pack_images pack_fonts pack_audio)

# Copy .fpk files to build/afterbloom/ (next to binary)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/images.fpk
        ${CMAKE_BINARY_DIR}/afterbloom/images.fpk
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/fonts.fpk
        ${CMAKE_BINARY_DIR}/afterbloom/fonts.fpk
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_BINARY_DIR}/audio.fpk
        ${CMAKE_BINARY_DIR}/afterbloom/audio.fpk
    COMMENT "Copying .fpk files to binary directory..."
)

if(WIN32)
  target_link_libraries(${PROJECT_NAME}
        PRIVATE winmm
    )
endif()
