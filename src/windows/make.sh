#! /bin/sh

# 获取当前项目所在目录
RUN_PATH=`pwd`
PROJECT_PATH=`echo $0 | grep "^/"`
if test "${PROJECT_PATH}"; then
        PROJECT_PATH==`dirname "$0"`
else
        PROJECT_PATH=`dirname "${RUN_PATH}/$0"`
fi

echo "Generating Makefile..."

# 生成Makefile文件
if [ ! -f "${PROJECT_PATH}/build/Makefile" ]; then
	rm -rf ${PROJECT_PATH}/build
	mkdir ${PROJECT_PATH}/build
	cd ${PROJECT_PATH}/build
	cmake -DCMAKE_TOOLCHAIN_FILE=../toolchain-mingw32.cmake ..
	if [ $? -ne 0 ]; then
        echo "cmake failed!"
        exit 2
	fi
	cd -
fi

echo "Compiling..."

# 编译
cd ${PROJECT_PATH}/build
make
if [ $? -ne 0 ]; then
        echo "make failed!"
        exit 3
fi
cd -

sz ${PROJECT_PATH}/build/psychokinesis.exe
