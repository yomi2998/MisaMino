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
endif()

set(DLLAI_SOURCES
  dllai/ai.cpp
  dllai/genmove.cpp
  dllai/tetris_gem.cpp
  dllai/dllai.cpp
)

add_library(misamino_ai SHARED ${DLLAI_SOURCES})
target_include_directories(misamino_ai PRIVATE dllai)
set_target_properties(misamino_ai PROPERTIES OUTPUT_NAME misamino_ai)
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
  add_executable(${harness} stress/${harness}.cpp $<TARGET_OBJECTS:dllai_objs>)
  target_include_directories(${harness} PRIVATE dllai)
endforeach()

add_executable(app_ai_stress stress/app_ai_stress.cpp $<TARGET_OBJECTS:tetris_ai_objs>)
target_include_directories(app_ai_stress PRIVATE tetris_ai)
target_link_libraries(app_ai_stress PRIVATE Threads::Threads)
if(UNIX)
  target_link_libraries(app_ai_stress PRIVATE dl)
endif()

foreach(harness sim_final sim_heavy sim_race)
  add_executable(${harness} stress/${harness}.cpp)
endforeach()
