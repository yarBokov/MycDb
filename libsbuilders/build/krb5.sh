#!/bin/bash

LIB_NAME="krb5"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

make_build_directory

cd $BUILD_DIR || check_errors

SRC_DIR="$LIB_DIR/src"
autoreconf -v $SRC_DIR || check_errors

$SRC_DIR/configure \
  CFLAGS="-m64 -fPIC $DEBUG_FLAG" \
  CXXFLAGS="-m64 -fPIC $DEBUG_FLAG" \
  LDFLAGS="-Wl,--allow-multiple-definition -ldl" \
  --enable-static \
  --disable-shared \
  --without-keyutils \
  || check_errors

make -C $BUILD_DIR -j$(nproc) || check_errors

make_out_directory

cp -r $BUILD_DIR/include $OUT_DIR/ || check_errors
cp $BUILD_DIR/lib/*.a $OUT_DIR/ || check_errors

remove_build_directory

build_end_notice