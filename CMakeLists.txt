cmake_minimum_required(VERSION 3.18)

project(mycelliaDB 
        LANGUAGES C CXX
        DESCRIPTION "Mycellia Database Prototype")

include("cmake/utils.cmake")
include("cmake/build.cmake")
include("cmake/libs.cmake")

include(GoogleTest)
enable_testing()

add_subdirectory(src)