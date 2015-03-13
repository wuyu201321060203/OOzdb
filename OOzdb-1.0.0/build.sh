#!/bin/sh

set -x

SOURCE_DIR=`pwd`
BUILD_DIR=${BUILD_DIR:-./build}
INSTALL_DIR=${INSTALL_DIR:-./install}
BUILD_NO_EXAMPLES=${BUILD_NO_EXAMPLES:-0}

mkdir $BUILD_DIR\
  && cd $BUILD_DIR\
  && cmake \
           -DCMAKE_INSTALL_PREFIX=$INSTALL_DIR \
           -DCMAKE_BUILD_NO_EXAMPLES=$BUILD_NO_EXAMPLES \
           $SOURCE_DIR \
  && make $*\
  && make install
