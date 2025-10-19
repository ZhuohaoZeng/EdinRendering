cmake_minimum_required(VERSION 3.16)
project(SOFTRT LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Warnings
if(MSVC)
  add_compile_options(/W4 /permissive- /MP)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -O2)
endif()

find_package(nlohmann_json CONFIG REQUIRED)
add_executable(main main.cpp JSONReader.cpp BVH.cpp)
target_link_libraries(main PRIVATE nlohmann_json::nlohmann_json)
