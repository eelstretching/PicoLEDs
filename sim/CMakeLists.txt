# Builds PicoLEDs and its examples for your desktop, so you can watch an
# animation in a window (or record it to a GIF) before putting it on a Pico.
# See README.md in this directory.
#
# This is its own CMake project, separate from the Pico build:
#
#   cmake -S sim -B build-sim
#   cmake --build build-sim -j
#   ./build-sim/FireDemo
#
# Another project (MegaTree, say) can simulate its own programs with:
#
#   add_subdirectory(path/to/PicoLEDs/sim picoleds-sim)
#   picoleds_sim_program(LeapingArches LeapingArches.cpp ...)

cmake_minimum_required(VERSION 3.13)

project(PicoLEDsSim
  DESCRIPTION "Desktop simulator for PicoLEDs"
  LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(PICOLEDS_DIR ${CMAKE_CURRENT_LIST_DIR}/.. CACHE PATH "The PicoLEDs checkout")
get_filename_component(PICOLEDS_DIR ${PICOLEDS_DIR} ABSOLUTE)
set(PICOLEDS_SIM_DIR ${CMAKE_CURRENT_LIST_DIR})

# The library itself, built from the same sources as the Pico build, except
# that the PIO+DMA Renderer is swapped for one that hands frames to the
# simulator.
file(GLOB PICOLEDS_SOURCES CONFIGURE_DEPENDS ${PICOLEDS_DIR}/src/*.cpp)
list(REMOVE_ITEM PICOLEDS_SOURCES ${PICOLEDS_DIR}/src/Renderer.cpp)
add_library(PicoLEDsSim STATIC ${PICOLEDS_SOURCES} ${PICOLEDS_SIM_DIR}/SimRenderer.cpp)
target_include_directories(PicoLEDsSim PUBLIC
  ${PICOLEDS_SIM_DIR}/stubs
  ${PICOLEDS_SIM_DIR}
  ${PICOLEDS_DIR}/include
  ${PICOLEDS_DIR}/src
)
target_compile_definitions(PicoLEDsSim PUBLIC PICOLEDS_SIM=1)
# The library is written for a 32 bit microcontroller and a compiler that
# doesn't mind much; don't bury real problems in warnings about that.
target_compile_options(PicoLEDsSim PUBLIC -w)

# The simulator: our main(), the clock, the GIF writer, and a window if SDL2 is
# around.
find_package(SDL2 CONFIG QUIET)
if(NOT TARGET SDL2::SDL2)
  find_package(PkgConfig QUIET)
  if(PKG_CONFIG_FOUND)
    pkg_check_modules(SDL2 QUIET IMPORTED_TARGET sdl2)
    if(TARGET PkgConfig::SDL2)
      add_library(SDL2::SDL2 ALIAS PkgConfig::SDL2)
    endif()
  endif()
endif()

set(SIM_SOURCES ${PICOLEDS_SIM_DIR}/Sim.cpp ${PICOLEDS_SIM_DIR}/GifWriter.cpp)
if(TARGET SDL2::SDL2)
  message(STATUS "PicoLEDs sim: SDL2 found, programs will open a window")
  list(APPEND SIM_SOURCES ${PICOLEDS_SIM_DIR}/SimWindowSDL.cpp)
else()
  message(STATUS "PicoLEDs sim: no SDL2 (brew install sdl2), programs can only record GIFs")
  list(APPEND SIM_SOURCES ${PICOLEDS_SIM_DIR}/SimWindowNone.cpp)
endif()
add_library(PicoLEDsSimMain OBJECT ${SIM_SOURCES})
target_link_libraries(PicoLEDsSimMain PUBLIC PicoLEDsSim)
find_package(Threads REQUIRED)
target_link_libraries(PicoLEDsSimMain PUBLIC Threads::Threads)
if(TARGET SDL2::SDL2)
  target_link_libraries(PicoLEDsSimMain PUBLIC SDL2::SDL2)
endif()

# picoleds_sim_program(NAME source.cpp ...)
#
# Builds a Pico program for the desktop. The sources are compiled as they are,
# except that their main() is renamed so the simulator can run it.
function(picoleds_sim_program NAME)
  add_executable(${NAME} ${ARGN} $<TARGET_OBJECTS:PicoLEDsSimMain>)
  target_link_libraries(${NAME} PRIVATE PicoLEDsSimMain)
  target_compile_definitions(${NAME} PRIVATE main=picoleds_user_main)
endfunction()

# When another project pulls this in with add_subdirectory() to simulate its
# own programs, it probably doesn't want all of our examples too.
if(NOT CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  return()
endif()

# The examples that make sense without real hardware. Ones that only exist on
# some branches are skipped when they aren't there.
set(PICOLEDS_SIM_EXAMPLES
  FireDemo:Fire/FireDemo.cpp
  Fireworks:Fireworks/Fireworks.cpp
  Fireworks2D:Fireworks/Fireworks2D.cpp
  MegaTree:MegaTree/MegaTree.cpp
  MicroTree:MegaTree/MicroTree.cpp
  CanvasTest:MegaTree/CanvasTest.cpp
  AniTest:AniTest/AniTest.cpp
  PatternTest:AniTest/PatternTest.cpp
  Fader:AniTest/Fader.cpp
  Fonts:Fonts/Fonts.cpp
  PanelTest:PanelTest/PanelTest.cpp
  SimpleLEDs:SimpleLEDs/SimpleLEDs.cpp
  StripTest:StripTest/StripTest.cpp
)
foreach(EXAMPLE ${PICOLEDS_SIM_EXAMPLES})
  string(REPLACE ":" ";" PARTS ${EXAMPLE})
  list(GET PARTS 0 NAME)
  list(GET PARTS 1 SOURCE)
  if(EXISTS ${PICOLEDS_DIR}/examples/${SOURCE})
    picoleds_sim_program(${NAME} ${PICOLEDS_DIR}/examples/${SOURCE})
  endif()
endforeach()

# Anything you drop in sim/programs/ gets built too: one .cpp with a main()
# per program, named after the file.
file(GLOB SIM_PROGRAMS CONFIGURE_DEPENDS ${PICOLEDS_SIM_DIR}/programs/*.cpp)
foreach(PROGRAM ${SIM_PROGRAMS})
  get_filename_component(NAME ${PROGRAM} NAME_WE)
  picoleds_sim_program(${NAME} ${PROGRAM})
endforeach()

# Builds every animation, runs it, deletes it, and checks it freed all its
# memory. See tests/EffectLifecycle.cpp. It has its own main(), so it doesn't
# use the simulator's.
add_executable(EffectLifecycle ${PICOLEDS_SIM_DIR}/tests/EffectLifecycle.cpp)
target_link_libraries(EffectLifecycle PRIVATE PicoLEDsSim)
include(CheckCXXSourceCompiles)
set(CMAKE_REQUIRED_FLAGS -fsanitize=address)
set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address)
check_cxx_source_compiles("int main() { return 0; }" PICOLEDS_SIM_HAVE_ASAN)
unset(CMAKE_REQUIRED_FLAGS)
unset(CMAKE_REQUIRED_LINK_OPTIONS)
if(PICOLEDS_SIM_HAVE_ASAN)
  target_compile_options(EffectLifecycle PRIVATE -fsanitize=address -fno-omit-frame-pointer)
  target_link_options(EffectLifecycle PRIVATE -fsanitize=address)
endif()
