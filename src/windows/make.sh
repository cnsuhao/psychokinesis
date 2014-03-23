#!/bin/sh

PROJECT_PATH=$HOME/test/windows

cd ${PROJECT_PATH}
if [ $? -ne 0 ]; then
	echo "${PROJECT_PATH} isn't exist!"
	exit 1
fi

if [ -d "${PROJECT_PATH}/build" ]; then
	echo "delete the directory: ${PROJECT_PATH}/build"
	rm -rf ${PROJECT_PATH}/build
fi

mkdir ${PROJECT_PATH}/build
cd ${PROJECT_PATH}/build
cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
if [ $? -ne 0 ]; then
        echo "cmake failed!"
        exit 2
fi

make
if [ $? -ne 0 ]; then
        echo "make failed!"
        exit 3
fi

strip psychokinesis.exe
sz psychokinesis.exe
