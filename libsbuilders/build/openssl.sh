#!/bin/bash

LIB_NAME="openssl"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

if [ "$DEBUG" = true ]
then
  DEBUG_FLAGS="--debug"
fi

cd $LIB_DIR || check_errors

# There is no `Makefile` in the repository,
# so `make` will return an error unless we run `config` atleast once
if [ -f $LIB_DIR/Makefile ]; then
  make clean -C $LIB_DIR || check_errors
fi

DEVRANDOM="\""\\\"/dev/urandom\\\"\"

$LIB_DIR/config \
  -DDEVRANDOM=$DEVRANDOM \
  --prefix=$BUILD_DIR \
  --openssldir=$BUILD_DIR \
  $DEBUG_FLAGS \
  -m64 no-shared no-tests -std=gnu17 -DPEDANTIC \
  || check_errors

make -C $LIB_DIR -j$(nproc) || check_errors

make_out_directory

cp -f $LIB_DIR/libcrypto.a $OUT_DIR/ || check_errors
cp -f $LIB_DIR/libssl.a $OUT_DIR/ || check_errors
cp -r $LIB_DIR/include $OUT_DIR/include || check_errors

remove_build_directory

make clean -C $LIB_DIR || check_errors

build_end_notice