#!/bin/bash

ROOT_DIR="$(cd $(dirname $0); cd ..; pwd -P)"
echo $ROOT_DIR
. $ROOT_DIR/build/core.sh

parse_arguments $@

#order of files is crucial
LIB_BUILD_SCRIPTS=(
  fmt.sh
  spdlog.sh
  cryptopp.sh
  zlib.sh
  boost.sh
  openssl.sh
  krb5.sh
  grpc.sh 
  googletest.sh
  libssh.sh
  pugixml.sh
)

for LIB_BUILD_FILE in ${LIB_BUILD_SCRIPTS[@]}; do
  LIB_NAME="${LIB_BUILD_FILE%.sh}"
  BUILD_SCRIPTS_DIR="$ROOT_DIR/$BUILD_DIR_NAME"
  if [ -f $BUILD_SCRIPTS_DIR/$LIB_BUILD_FILE ]; then
    $BUILD_SCRIPTS_DIR/$LIB_BUILD_FILE $@ || check_errors
  else
    echo "The '$LIB_NAME' is in the library list, but the '$LIB_BUILD_FILE' is missing."
    exit 1
  fi
done

echo "*************************************************************"
echo "All aux libraries have been successfully built."
echo -e "**********************************************************\n"