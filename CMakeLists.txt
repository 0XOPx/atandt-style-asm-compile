cmake_minimum_required(VERSION 3.20)

project(ATandTStyleASMCompile VERSION 0.1.0 LANGUAGES CXX)

add_executable(atandt-compile
    src/main.cpp
)

target_compile_features(atandt-compile PRIVATE cxx_std_17)

if(MSVC)
    target_compile_options(atandt-compile PRIVATE /W4 /permissive-)
else()
    target_compile_options(atandt-compile PRIVATE -Wall -Wextra -Wpedantic)
endif()

set_target_properties(atandt-compile PROPERTIES
    CXX_EXTENSIONS OFF
    OUTPUT_NAME "atandt-compile"
)
