cmake_minimum_required(VERSION 3.16)
project(lab3 LANGUAGES CXX)

add_executable(lab3 main.cpp sample.h)
add_executable(lab3_stage1 stage1/main.cpp stage1/sample_double.cpp stage1/sample_double.h)

foreach(target IN ITEMS lab3 lab3_stage1)
    set_target_properties(${target} PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
        CXX_EXTENSIONS OFF
    )

    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /utf8)
    else()
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wpedantic)
    endif()
endforeach()
