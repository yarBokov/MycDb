#!/bin/bash

LIB_DIR="$ROOT_DIR/$LIB_NAME"

BUILD_DIR_NAME="build"
BUILD_DIR="$LIB_DIR/$BUILD_DIR_NAME"

OUT_DIR_NAME="lib64"
OUT_DIR="$LIB_DIR/$OUT_DIR_NAME"

function check_errors
{
  EXIT_CODE=$?
  if [ $EXIT_CODE <> 0 ]; then
    ERROR_MESSAGE_SUFFIX=""
    if [ -n $LIB_NAME ]; then
      ERROR_MESSAGE_SUFFIX=" when building $LIB_NAME"
    fi
    echo "Error in file ${0} on line ${BASH_LINENO[0]}$ERROR_MESSAGE_SUFFIX."
    exit $EXIT_CODE
  fi
}

function parse_arguments
{
  BUILD_TYPE=release
  local cmake_build_type_value=Release
  if [[ "${1}" == "--debug" ]]; then
    DEBUG=true
    BUILD_TYPE=debug
    DEBUG_FLAG="-g"
    cmake_build_type_value=Debug
  fi

  CMAKE_BUILD_TYPE="-DCMAKE_BUILD_TYPE=$cmake_build_type_value"
}

function build_start_notice
{
  echo "*************************************************************"
  echo "Building $LIB_NAME."
  echo "*************************************************************"
}

function build_end_notice
{
  echo "*************************************************************"
  echo "The $LIB_NAME library has been built. Output directory:"
  echo "$OUT_DIR"
  echo -e "*************************************************************\n"
}

function remove_build_directory
{
  rm -rf $BUILD_DIR || check_errors
}

function make_build_directory
{
  remove_build_directory
  mkdir -p $BUILD_DIR || check_errors
}

function make_out_directory
{
  rm -rf $OUT_DIR || check_errors
  mkdir -p $OUT_DIR || check_errors
}

function copy_libs_to_out_directory
{
  make_out_directory
  
  LIB_LIST=$(find $LIB_DIR -name *.a)
  for LIB in ${LIB_LIST[@]}; do
    cp $LIB $OUT_DIR/ || check_errors
  done
}
