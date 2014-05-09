#!/bin/sh

ICONV_PATH=$HOME/libiconv-1.14
INSTALL_PATH_PREFIX=$HOME/mingw32

COMPILER_PREFIX=i686-w64-mingw32

if [ -d "${ICONV_PATH}" ]; then
        echo "delete the directory: ${ICONV_PATH}"
        rm -rf ${ICONV_PATH}
fi

tar xzf ${ICONV_PATH}.tar.gz
cd ${ICONV_PATH}

./configure --prefix=${INSTALL_PATH_PREFIX} --host=${COMPILER_PREFIX} --enable-static
make -j 4
make install

cd ..
rm -rf ${ICONV_PATH}
