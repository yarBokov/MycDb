#!/bin/bash

LIB_NAME="googletest"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

make_build_directory

cd $LIB_DIR || check_errors

cmake \
  -B $BUILD_DIR \
  -S $LIB_DIR \
  -DCMAKE_CXX_FLAGS="-m64 -std=c++17 -fPIC" \
  -DCMAKE_C_FLAGS="-m64 -fPIC" \
  $CMAKE_BUILD_TYPE \
  || check_errors

cmake --build $BUILD_DIR -j$(nproc) || check_errors

copy_libs_to_out_directory

remove_build_directory

build_end_notice