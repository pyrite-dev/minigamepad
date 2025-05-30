project(minigamepad)

FILE(GLOB_RECURSE sources src/common/*.c src/linux/*.c)

add_library(${PROJECT_NAME} ${sources})

include_directories(include src/common)

# go through each c file in the examples folder and compile it to an example
make_directory(${CMAKE_BINARY_DIR}/examples)
file(GLOB files "${CMAKE_SOURCE_DIR}/examples/*.c")
foreach(file ${files})
    cmake_path(GET file FILENAME program_name_full)
    cmake_path(GET file EXTENSION program_ext)
    string(REPLACE ${program_ext} "" program_name ${program_name_full})
    message("example: "${program_name})
    add_executable(${CMAKE_BINARY_DIR}/examples/${program_name} ${file})
    target_link_libraries(${CMAKE_BINARY_DIR}/examples/${program_name} ${PROJECT_NAME})
endforeach()
