cmake_minimum_required(VERSION 3.17)

find_package(Drogon REQUIRED)

add_executable(server main.cpp)

target_compile_features(server PRIVATE cxx_std_20)

target_link_libraries(server PRIVATE Drogon::Drogon)
