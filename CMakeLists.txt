cmake_minimum_required(VERSION 3.20)
project(misamino CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

if(MSVC)
  add_compile_options(/utf-8)
  set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

set(DLLAI_SOURCES
  dllai/ai.cpp
  dllai/genmove.cpp
  dllai/tetris_gem.cpp
  dllai/dllai.cpp
)

add_library(misamino_ai SHARED ${DLLAI_SOURCES})
target_include_directories(misamino_ai PRIVATE dllai)
if(WIN32)
  set_target_properties(misamino_ai PROPERTIES OUTPUT_NAME dllai)
else()
  set_target_properties(misamino_ai PROPERTIES OUTPUT_NAME misamino_ai)
endif()
if(NOT WIN32)
  set_target_properties(misamino_ai PROPERTIES POSITION_INDEPENDENT_CODE ON C_VISIBILITY_PRESET default)
endif()

add_library(dllai_objs OBJECT ${DLLAI_SOURCES})
target_include_directories(dllai_objs PUBLIC dllai)

add_library(tetris_ai_objs OBJECT
  tetris_ai/gamepool.cpp
  tetris_ai/genmove.cpp
  tetris_ai/tetris_ai.cpp
  tetris_ai/tetris_gem.cpp
)
target_include_directories(tetris_ai_objs PUBLIC tetris_ai ${CMAKE_CURRENT_SOURCE_DIR}/stress)

add_executable(headless_tetris_ai tetris_ai/headless_main.cpp $<TARGET_OBJECTS:tetris_ai_objs>)
target_include_directories(headless_tetris_ai PRIVATE tetris_ai)
find_package(Threads REQUIRED)
target_link_libraries(headless_tetris_ai PRIVATE Threads::Threads)
if(UNIX)
  target_link_libraries(headless_tetris_ai PRIVATE dl)
endif()

foreach(harness ub_stress upcome_behavior mini_repro)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stress/${harness}.cpp)
    add_executable(${harness} stress/${harness}.cpp $<TARGET_OBJECTS:dllai_objs>)
    target_include_directories(${harness} PRIVATE dllai)
  endif()
endforeach()

if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stress/app_ai_stress.cpp)
  add_executable(app_ai_stress stress/app_ai_stress.cpp $<TARGET_OBJECTS:tetris_ai_objs>)
  target_include_directories(app_ai_stress PRIVATE tetris_ai)
  target_link_libraries(app_ai_stress PRIVATE Threads::Threads)
  if(UNIX)
    target_link_libraries(app_ai_stress PRIVATE dl)
  endif()
endif()

foreach(harness sim_final sim_heavy sim_race)
  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/stress/${harness}.cpp)
    add_executable(${harness} stress/${harness}.cpp)
  endif()
endforeach()

option(MISAMINO_BUILD_APP "Build the Tspin game (SDL2 graphics + miniaudio sound)" ON)

if(MISAMINO_BUILD_APP)
  if(POLICY CMP0169)
    cmake_policy(SET CMP0169 OLD)
  endif()
  set(CMAKE_POLICY_VERSION_MINIMUM 3.10)
  include(FetchContent)
  set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(SDL_SHARED OFF CACHE BOOL "" FORCE)
  set(SDL_STATIC ON CACHE BOOL "" FORCE)
  set(SDL_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_Declare(SDL2
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/libsdl-org/SDL/archive/refs/tags/release-2.32.10.tar.gz
    URL_HASH SHA256=03f9d7c191a837525c9cda6406af2f2e48be02b5e7eb03d949cc9f1e9ca41c8b)
  FetchContent_Declare(FreeType
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/freetype/freetype/archive/refs/tags/VER-2-13-3.tar.gz
    URL_HASH SHA256=bc5c898e4756d373e0d991bab053036c5eb2aa7c0d5c67e8662ddc6da40c4103)
  FetchContent_Declare(SDL2_ttf
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/libsdl-org/SDL_ttf/archive/refs/tags/release-2.20.2.tar.gz
    URL_HASH SHA256=0fe9d587cdc4e6754b647536d0803bea8ca6ac77146c4209e0bed22391cf8241)
  set(SDL2TTF_VENDORED ON CACHE BOOL "" FORCE)
  set(SDL2TTF_BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
  set(SDL2TTF_HARFBUZZ OFF CACHE BOOL "" FORCE)
  set(SDL2TTF_SAMPLES OFF CACHE BOOL "" FORCE)
  FetchContent_Populate(FreeType)
  FetchContent_MakeAvailable(SDL2)
  if(NOT MSVC)
    foreach(sdl_lib_target SDL2 SDL2-static SDL2-shared)
      if(TARGET ${sdl_lib_target})
        target_compile_options(${sdl_lib_target} PRIVATE -Wno-discarded-qualifiers)
      endif()
    endforeach()
  endif()
  FetchContent_Populate(SDL2_ttf)
  if(NOT EXISTS "${sdl2_ttf_SOURCE_DIR}/external/freetype/CMakeLists.txt")
    file(COPY "${freetype_SOURCE_DIR}/." DESTINATION "${sdl2_ttf_SOURCE_DIR}/external/freetype")
  endif()
  add_subdirectory(${sdl2_ttf_SOURCE_DIR} ${sdl2_ttf_BINARY_DIR})

  if(BUILD_SHARED_LIBS)
    set(TSPIN_TTF_TARGET SDL2_ttf::SDL2_ttf)
  else()
    set(TSPIN_TTF_TARGET SDL2_ttf::SDL2_ttf-static)
  endif()

  add_executable(Tspin
    tetris_ai/main.cpp
    tetris_ai/profile.cpp
    tetris_ai/sdl_graphics.cpp
    tetris_ai/miniaudio_impl.cpp
    $<TARGET_OBJECTS:tetris_ai_objs>
  )
  target_include_directories(Tspin PRIVATE tetris_ai)
  if(WIN32)
    target_sources(Tspin PRIVATE tetris_ai/tetris_ai.rc)
    enable_language(RC)
  endif()
  target_link_libraries(Tspin PRIVATE SDL2::SDL2-static ${TSPIN_TTF_TARGET} Threads::Threads)
  if(UNIX)
    target_link_libraries(Tspin PRIVATE dl)
  endif()
  add_custom_command(TARGET Tspin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
      ${CMAKE_CURRENT_SOURCE_DIR}/tetris_ai/data
      $<TARGET_FILE_DIR:Tspin>/data)
  add_custom_command(TARGET Tspin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      ${CMAKE_CURRENT_SOURCE_DIR}/tetris_ai/bin/misamino.ini
      $<TARGET_FILE_DIR:Tspin>/misamino.ini)
  add_dependencies(Tspin misamino_ai)
  add_custom_command(TARGET Tspin POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_FILE:misamino_ai>
      $<TARGET_FILE_DIR:Tspin>/$<TARGET_FILE_NAME:misamino_ai>)
endif()
