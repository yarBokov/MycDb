#!/bin/bash

LIB_NAME="libssh"
ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
. $ROOT_DIR/build/core.sh

parse_arguments $@
build_start_notice

make_build_directory

cd $LIB_DIR || check_errors

OPENSSL_DIR="$ROOT_DIR/openssl"
OPENSSL_OUT_DIR="$OPENSSL_DIR/$OUT_DIR_NAME"
ZLIB_OUT_DIR="$ROOT_DIR/zlib/$OUT_DIR_NAME"
KRB5_OUT_DIR="$ROOT_DIR/krb5/$OUT_DIR_NAME"

cmake \
  -B $BUILD_DIR \
  -S $LIB_DIR \
  $CMAKE_BUILD_TYPE \
  -DCMAKE_C_FLAGS="-fPIC" \
  -DCMAKE_CXX_FLAGS="-fPIC" \
  -DOPENSSL_CRYPTO_LIBRARY=$OPENSSL_OUT_DIR/libcrypto.a \
  -DOPENSSL_INCLUDE_DIR=$OPENSSL_OUT_DIR/include \
  -DOPENSSL_ROOT_DIR=$OPENSSL_DIR \
  -DZLIB_LIBRARY_RELEASE=$ZLIB_OUT_DIR/libz.a \
  -DZLIB_INCLUDE_DIR=$ZLIB_OUT_DIR \
  -DCOM_ERR_LIBRARY=$KRB5_OUT_DIR/libcom_err.a \
  -DGSSAPI_LIBRARY=$KRB5_OUT_DIR/libgssapi_krb5.a \
  -DK5CRYPTO_LIBRARY=$KRB5_OUT_DIR/libk5crypto.a \
  -DKRB5_LIBRARY=$KRB5_OUT_DIR/libkrb5.a \
  -DBUILD_STATIC_LIB=ON \
  -DWITH_NACL=OFF \
  -DWITH_DEBUG_CALLTRACE=OFF \
  -DWITH_EXAMPLES=OFF \
  || check_errors

cmake --build $BUILD_DIR -j$(nproc)|| check_errors

copy_libs_to_out_directory

OUT_INC_DIR="$OUT_DIR/include/libssh"
mkdir -p $OUT_INC_DIR || check_errors
cp $BUILD_DIR/include/libssh/libssh_version.h $OUT_INC_DIR/ || check_errors

remove_build_directory

rm $LIB_DIR/compile_commands.json || check_errors

build_end_notice