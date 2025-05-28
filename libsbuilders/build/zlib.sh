#!/bin/bash

LIB_NAME="zlib"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

cd $LIB_DIR || check_errors

# Returns an error if the command is called before the first build of the
# library. For example, if you download a new version via pull script.
# make clean -C $LIB_DIR || check_errors

# This is one command, CFLAGS is needed!
CC=`which gcc` \
CFLAGS="-m64 -std=c17 -fPIC $DEBUG_FLAG" \
  $LIB_DIR/configure --static || check_errors

make -C $LIB_DIR -j$(nproc) || check_errors

copy_libs_to_out_directory

#make clean -C $LIB_DIR || check_errors

build_end_notice
