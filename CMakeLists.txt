cmake_minimum_required(VERSION 3.20)

project(ArgumentParser VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-Wall -Wextra -Wpedantic)

if(CMAKE_BUILD_TYPE MATCHES Debug)
    add_compile_options(-fsanitize=address,undefined -g)
    add_link_options(-fsanitize=address,undefined)
endif()

add_subdirectory(lib)
add_subdirectory(bin)

enable_testing()
add_subdirectory(tests)