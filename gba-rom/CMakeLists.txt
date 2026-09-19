cmake_minimum_required(VERSION 3.21)
project(gba_gba-player LANGUAGES CXX)

set(GBA_ROM gba-player)

add_executable(
    ${GBA_ROM}
    src/main.cc
    src/frame_buffer.cc
)

add_subdirectory(lib)

target_include_directories(${GBA_ROM} PUBLIC include)
target_compile_options(${GBA_ROM} PRIVATE -mthumb -fconserve-stack -fomit-frame-pointer)
target_link_libraries(${GBA_ROM} PRIVATE librom libseven)
    
# ROM header info
set_target_properties(${GBA_ROM} PROPERTIES
    ROM_TITLE "GBA Player"
    ROM_ID AABE
    ROM_MAKER AF
    ROM_VERSION 1
)

# install to .gba
install_rom(${GBA_ROM})

add_custom_target(rom ALL
    COMMAND ${CMAKE_COMMAND} --install ${CMAKE_BINARY_DIR}
    DEPENDS gba-player
)