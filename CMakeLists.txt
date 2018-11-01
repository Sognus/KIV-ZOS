cmake_minimum_required(VERSION 3.12)
project(KIV_ZOS C)

set(CMAKE_C_STANDARD 99)

add_executable(KIV_ZOS main.c structure.c ntfs.c ntfs.h)