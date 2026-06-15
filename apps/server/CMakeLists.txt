cmake_minimum_required(VERSION 3.20)

# Conan-generated toolchain/find modules
if(EXISTS "${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    include("${CMAKE_BINARY_DIR}/conanbuildinfo.cmake")
    conan_basic_setup(TARGETS)
endif()

find_package(Drogon REQUIRED)

add_executable(server main.cpp)

target_compile_features(server PRIVATE cxx_std_20)

target_link_libraries(server PRIVATE Drogon::Drogon)
