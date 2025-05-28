#!/bin/bash

LIB_NAME="fmt"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

cd $LIB_DIR || check_errors

make_build_directory

cmake \
  -B $BUILD_DIR \
  -S $LIB_DIR \
  $CMAKE_BUILD_TYPE \
  -DCMAKE_POSITION_INDEPENDENT_CODE=TRUE \
  -DFMT_TEST=OFF \
  || check_errors

cmake --build $BUILD_DIR -j$(nproc)|| check_errors

copy_libs_to_out_directory

remove_build_directory

build_end_notice