cmake_minimum_required(VERSION 3.0.0)
project(ics-linker LANGUAGES C CXX)


add_executable(ics-linker 
    src/main.cc
    src/util.cc
    src/util.h
    src/relocation.cc
    src/relocation.h
    src/resolve.cc
    src/resolve.h
)

target_compile_options(ics-linker PRIVATE
    -Wall
    -Wextra
)