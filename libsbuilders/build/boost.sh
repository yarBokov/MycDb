#!/bin/bash

LIB_NAME="boost"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

ZLIB_DIR="$ROOT_DIR/zlib/"

cd $LIB_DIR || check_errors

git submodules update --init

make_build_directory

$LIB_DIR/bootstrap.sh \
  --without-libraries=graph,graph_parallel,mpi,fiber,python \
  || check_errors

COMMON_PROPS_B2=" \
  --build-dir=$BUILD_DIR \
  variant=$BUILD_TYPE \
  -sZLIB_SOURCE=$ZLIB_DIR \
  toolset=gcc \
  link=static \
  threading=multi \
  -q"

make_out_directory

$LIB_DIR/b2 \
  address-model=64 \
  $COMMON_PROPS_B2 \
  --stagedir=$OUT_DIR \
  cxxflags="-std=c++17 -fPIC" \
  || check_errors

remove_build_directory

build_end_notice