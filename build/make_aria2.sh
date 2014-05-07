#!/bin/sh

# set up the necessary complier environment variables
COMPILER_PREFIX=i686-w64-mingw32
export CC=$COMPILER_PREFIX-gcc
export CXX=$COMPILER_PREFIX-g++
export CPP=$COMPILER_PREFIX-cpp
export RANLIB=$COMPILER_PREFIX-ranlib

# set up the install pathname
INSTALL_PATH_PREFIX=$HOME/mingw32

# set up the source codes pathname
SOURCE_PREFIX=$HOME
C_ARES_PATH=${SOURCE_PREFIX}/c-ares-1.10.0
OPENSSL_PATH=${SOURCE_PREFIX}/openssl-1.0.1g
EXPAT_PATH=${SOURCE_PREFIX}/expat-2.1.0
ZLIB_PATH=${SOURCE_PREFIX}/zlib-1.2.8
CPPUNIT_PATH=${SOURCE_PREFIX}/cppunit-1.12.1
ARIA2_PATH=${SOURCE_PREFIX}/aria2-1.18.5

# check all the source codes tar.gz
if [ ! -f "${C_ARES_PATH}.tar.gz" ]; then
	echo "ERROR: ${C_ARES_PATH}.tar.gz does't exist!"
	exit 1
fi
if [ ! -f "${OPENSSL_PATH}.tar.gz" ]; then
	echo "ERROR: ${OPENSSL_PATH}.tar.gz does't exist!"
	exit 1
fi
if [ ! -f "${EXPAT_PATH}.tar.gz" ]; then
	echo "ERROR: ${EXPAT_PATH}.tar.gz does't exist!"
	exit 1
fi
if [ ! -f "${ZLIB_PATH}.tar.gz" ]; then
	echo "ERROR: ${ZLIB_PATH}.tar.gz does't exist!"
	exit 1
fi
if [ ! -f "${CPPUNIT_PATH}.tar.gz" ]; then
	echo "ERROR: ${CPPUNIT_PATH}.tar.gz does't exist!"
	exit 1
fi
if [ ! -f "${ARIA2_PATH}.tar.gz" ]; then
	echo "ERROR: ${ARIA2_PATH}.tar.gz does't exist!"
	exit 1
fi

# clear all the original source codes path
if [ -d "${C_ARES_PATH}" ]; then
	echo "delete the directory: ${C_ARES_PATH}"
	rm -rf ${C_ARES_PATH}
fi
if [ -d "${OPENSSL_PATH}" ]; then
	echo "delete the directory: ${OPENSSL_PATH}"
	rm -rf ${OPENSSL_PATH}
fi
if [ -d "${EXPAT_PATH}" ]; then
	echo "delete the directory: ${EXPAT_PATH}"
	rm -rf ${EXPAT_PATH}
fi
if [ -d "${ZLIB_PATH}" ]; then
	echo "delete the directory: ${ZLIB_PATH}"
	rm -rf ${ZLIB_PATH}
fi
if [ -d "${CPPUNIT_PATH}" ]; then
	echo "delete the directory: ${CPPUNIT_PATH}"
	rm -rf ${CPPUNIT_PATH}
fi
if [ -d "${ARIA2_PATH}" ]; then
	echo "delete the directory: ${ARIA2_PATH}"
	rm -rf ${ARIA2_PATH}
fi

# start compiling
echo "compiling c_ares..."
tar zxf ${C_ARES_PATH}.tar.gz
cd ${C_ARES_PATH}
./configure --prefix=${INSTALL_PATH_PREFIX} --host=${COMPILER_PREFIX} --enable-static
make;make install
cd ..
rm -rf ${C_ARES_PATH}

echo "compiling openssl..."
tar zxf ${OPENSSL_PATH}.tar.gz
cd ${OPENSSL_PATH}
./Configure --prefix=${INSTALL_PATH_PREFIX} mingw
make -j 4
make install
cd ..
rm -rf ${OPENSSL_PATH}

echo "compiling expat..."
tar zxf ${EXPAT_PATH}.tar.gz
cd ${EXPAT_PATH}
./configure --prefix=${INSTALL_PATH_PREFIX} --host=${COMPILER_PREFIX} --enable-static
make;make install
cd ..
rm -rf ${EXPAT_PATH}

echo "compiling zlib..."
tar zxf ${ZLIB_PATH}.tar.gz
cd ${ZLIB_PATH}
./configure --prefix=${INSTALL_PATH_PREFIX} --static
make;make install
cd ..
rm -rf ${ZLIB_PATH}

echo "compiling cppunit..."
tar zxf ${CPPUNIT_PATH}.tar.gz
cd ${CPPUNIT_PATH}
./configure --prefix=${INSTALL_PATH_PREFIX} --host=${COMPILER_PREFIX} --enable-static
make;make install
cd ..
rm -rf ${CPPUNIT_PATH}

echo "compiling aria2..."
tar zxf ${ARIA2_PATH}.tar.gz
cd ${ARIA2_PATH}
./configure --host=${COMPILER_PREFIX} --prefix=${INSTALL_PATH_PREFIX} --enable-static --enable-libaria2 --without-included-gettext --disable-nls --with-libcares="/lib/i386-linux-gnu" --without-gnutls --with-openssl="${INSTALL_PATH_PREFIX}/include/openssl" --without-sqlite3  --without-libxml2 --with-libexpat --with-libz --without-libgcrypt  --without-libnettle --with-cppunit-prefix=${INSTALL_PATH_PREFIX} CPPFLAGS="-I${INSTALL_PATH_PREFIX}/include" LDFLAGS="-L${INSTALL_PATH_PREFIX}/lib" PKG_CONFIG_PATH="${INSTALL_PATH_PREFIX}/lib/pkgconfig"
make -j 4
make install
cd ..
rm -rf ${ARIA2_PATH}

echo "successful!"
