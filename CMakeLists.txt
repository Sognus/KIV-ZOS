cmake_minimum_required(VERSION 2.7.2)
project(KIV_ZOS)

set(CMAKE_CXX_STANDARD 99)

include_directories(.)

set ( CMAKE_C_FLAGS "-lm")

add_executable(KIV_ZOS
        main.c
        ntfs.c
        ntfs.h
        structure.c
        structure.h ntfs_commands.c ntfs_commands.h shell.c shell.h ntfs_logic.c ntfs_logic.h path_logic.c path_logic.h)

target_link_libraries(KIV_ZOS m)
