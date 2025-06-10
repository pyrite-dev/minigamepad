project(minigamepad)
cmake_minimum_required(VERSION 3.13) # 3.13 is the last version which will run on Windows XP

option(BUILD_EXAMPLES "Build with examples" ON)

# Set platform specific sources and links
IF(WIN32)
SET(MG_NO_STDTYPES true)
SET(PLATFORM_SOURCES src/windows/*.c)
elseif(UNIX)
SET(MG_NO_STDTYPES false)
SET(PLATFORM_SOURCES src/linux/*.c)
# X11
if(BUILD_EXAMPLES)
find_package(X11)
if(NOT X11_FOUND)
message( FATAL_ERROR "CMake did not find x11" )
endif()
if(NOT X11_Xrandr_FOUND)
message( FATAL_ERROR "CMake did not find xrandr" )
endif()
find_package(OpenGL)
if(NOT OPENGL_FOUND)
message( FATAL_ERROR "CMake did not find OpenGL" )
endif()
link_libraries(GL X11 Xrandr)
endif()
ENDIF()

# Combine them with all the sources
FILE(GLOB_RECURSE sources src/common/*.c src/include/minigamepad.h ${PLATFORM_SOURCES})

add_library(${PROJECT_NAME} ${sources})
IF(WIN32)
add_compile_definitions(${PROJECT_NAME} MG_NO_STDTYPES)
endif()

include_directories(include src/common)

if(BUILD_EXAMPLES)
link_libraries(m)
# go through each c file in the examples folder and compile it to an example
make_directory(${CMAKE_BINARY_DIR}/examples)
file(GLOB files "${CMAKE_SOURCE_DIR}/examples/*.c")
foreach(file ${files})
    get_filename_component(program_name_full ${file} NAME_WE)
    get_filename_component(program_ext ${file} EXT)
    string(REPLACE ${program_ext} "" program_name ${program_name_full})
    message("example: "${program_name})
    add_executable(${program_name} ${file})
    target_link_libraries(${program_name} ${PROJECT_NAME})
endforeach()
endif()
