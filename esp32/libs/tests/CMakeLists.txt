cmake_minimum_required(VERSION 3.16)
project(udp_server_tests LANGUAGES CXX)

enable_testing()

add_executable(udp_server_test
    udp_server_test.cpp
    ../src/udp_server.cpp
)
target_compile_features(udp_server_test PRIVATE cxx_std_17)
target_include_directories(udp_server_test PRIVATE ../include)
add_test(NAME udp_server_test COMMAND udp_server_test)