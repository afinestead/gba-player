cmake_minimum_required(VERSION 3.21)
project(gba_player LANGUAGES C)

add_executable(player src/main.c)

# gba-toolchain sets `CMAKE_SYSTEM_NAME` to `AdvancedGameBoy`
if(CMAKE_SYSTEM_NAME STREQUAL AdvancedGameBoy)
    find_package(librom REQUIRED) # ROM runtime
    find_package(agbabi REQUIRED) # Optimized library functions

    add_subdirectory(libs)

    target_compile_options(player PRIVATE -mthumb -fconserve-stack -fomit-frame-pointer)
    target_link_libraries(player PRIVATE librom libseven agbabi)
    
    # ROM header info
    set_target_properties(player PROPERTIES
        ROM_TITLE "My Game"
        ROM_ID AABE
        ROM_MAKER CD
        ROM_VERSION 1
    )

    # install to .gba
    install_rom(player)
endif()