#!/bin/bash

mkdir -p ../../aux_libs/build/
yes | cp -rf ./build/*.sh ../../aux_libs/build/
yes | cp -f ./build/fmt-config.cmake ../../aux_libs/