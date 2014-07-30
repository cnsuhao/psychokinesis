#!/bin/bash

# set up the necessary complier environment variables
COMPILER_PREFIX=i686-w64-mingw32
export CC=$COMPILER_PREFIX-gcc
export CXX=$COMPILER_PREFIX-g++
export CPP=$COMPILER_PREFIX-cpp
export RANLIB=$COMPILER_PREFIX-ranlib
export AR=$COMPILER_PREFIX-ar

# set up the install pathname
INSTALL_PATH_PREFIX=$HOME/mingw32
export CPPFLAGS="-I${INSTALL_PATH_PREFIX}/include"
export LDFLAGS="-L${INSTALL_PATH_PREFIX}/lib"

# set up the source codes pathname
GLOOX_PATH=$HOME/gloox-1.0.9

# check the source codes tar.bz2
if [ ! -f "${GLOOX_PATH}.tar.bz2" ]; then
        echo "ERROR: ${GLOOX_PATH}.tar.bz2 does't exist!"
        exit 1
fi

# clear the original source codes path
if [ -d "${GLOOX_PATH}" ]; then
        echo "delete the directory: ${GLOOX_PATH}"
        rm -rf ${GLOOX_PATH}
fi

# start compiling
echo "compiling gloox..."
tar xjf ${GLOOX_PATH}.tar.bz2
cd ${GLOOX_PATH}
./configure --prefix=${INSTALL_PATH_PREFIX} --host=${COMPILER_PREFIX} --enable-static --with-schannel --with-zlib --with-tests=no --with-examples=no
if [ $? -ne 0 ]; then
        echo "gloox: configure failed."
        exit 2
fi

make;make install
if [ $? -ne 0 ]; then
        echo "gloox: make failed."
        exit 2
fi

cd ..
rm -rf ${GLOOX_PATH}