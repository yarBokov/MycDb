cmake_minimum_required(VERSION 3.18)

project(mycdb 
        LANGUAGES CXX
        DESCRIPTION "Mycellia Database Prototype")

include("cmake/utils.cmake")
include("cmake/build.cmake")
include("cmake/libs.cmake")

# include(GoogleTest)
# enable_testing()

  if(NOT CMAKE_BUILD_TYPE)
    message(STATUS "Setting build type to 'RelWithDebInfo' as none was specified")
    set(CMAKE_BUILD_TYPE RelWithDebInfo)
  endif()

add_subdirectory(src)