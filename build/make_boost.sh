#!/bin/sh

BOOST_PATH=$HOME/boost_1_55_0
INSTALL_PATH_PREFIX=$HOME/mingw32

COMPILER_PREFIX=i686-w64-mingw32
export CC=$COMPILER_PREFIX-gcc
export CXX=$COMPILER_PREFIX-g++
export CPP=$COMPILER_PREFIX-cpp
export RANLIB=$COMPILER_PREFIX-ranlib

if [ -d "${BOOST_PATH}" ]; then
        echo "delete the directory: ${BOOST_PATH}"
        rm -rf ${BOOST_PATH}
fi

7zr x ${BOOST_PATH}.7z
cd ${BOOST_PATH}
./bootstrap.sh
echo "using gcc : mingw32 : i686-w64-mingw32-g++ : <rc>i686-w64-mingw32-windres <archiver>i686-w64-mingw32-ar <ranlib>i686-w64-mingw32-ranlib ;" >> ${BOOST_PATH}/tools/build/v2/user-config.jam
./b2 install toolset=gcc-mingw32 target-os=windows threading=multi threadapi=win32 link=static variant=release --prefix=${INSTALL_PATH_PREFIX} --without-mpi

cp -f ../lib/boost/property_tree/detail/json_parser_write.hpp ${INSTALL_PATH_PREFIX}/include/boost/property_tree/detail/

rm -rf ${BOOST_PATH}
