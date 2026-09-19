cmake_minimum_required(VERSION 3.21)
project(gba_gba-player LANGUAGES CXX)

add_executable(
    gba-player
    src/main.cc
)

add_subdirectory(lib)

target_compile_options(gba-player PRIVATE -mthumb -fconserve-stack -fomit-frame-pointer)
target_link_libraries(gba-player PRIVATE librom libseven)
    
# ROM header info
set_target_properties(gba-player PROPERTIES
    ROM_TITLE "GBA Player"
    ROM_ID AABE
    ROM_MAKER AF
    ROM_VERSION 1
)

# install to .gba
install_rom(gba-player)

add_custom_target(rom ALL
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
    DEPENDS gba-player
)