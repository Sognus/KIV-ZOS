cmake_minimum_required(VERSION 3.12)
project(KIV_ZOS)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)

add_executable(KIV_ZOS
        main.c
        ntfs.c
        ntfs.h
        structure.c
        structure.h ntfs_commands.c ntfs_commands.h shell.c shell.h ntfs_logic.c ntfs_logic.h)
