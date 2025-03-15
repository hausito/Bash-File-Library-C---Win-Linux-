# CMakeLists.txt
cmake_minimum_required(VERSION 3.10)
project(Tema_SO C)

# Add the library
add_library(so_file_lib STATIC so_file_lib.c)

# Add the executable
add_executable(main main.c)

# Link the library to the executable
target_link_libraries(main PRIVATE so_file_lib)

# Ensure that _WIN32 is defined if on Windows
if(WIN32)
    target_compile_definitions(so_file_lib PRIVATE _WIN32)
    target_compile_definitions(main PRIVATE _WIN32)
endif()