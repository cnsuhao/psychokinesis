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

# 复制编译依赖的库至lib目录
mkdir -p $HOME/mingw32/lib
cp -f libriched20.a $HOME/mingw32/lib
if [ $? -ne 0 ]; then
	echo "copy libriched20.a failed."
	exit 2
fi

# 编译
cd ${PROJECT_PATH}/build
make
if [ $? -ne 0 ]; then
        echo "make failed!"
        exit 3
fi
cd -

# 复制库文件到mingw目录
mkdir -p $HOME/mingw32/include/DuiLib
cp -f *.h $HOME/mingw32/include/DuiLib
if [ $? -ne 0 ]; then
	echo "copy header files failed."
	exit 4
fi

mkdir -p $HOME/mingw32/bin
cp -f build/libUIlib.dll.a $HOME/mingw32/lib
if [ $? -ne 0 ]; then
	echo "copy libUIlib.dll.a failed."
	exit 4
fi

cp -f build/libUIlib.dll $HOME/mingw32/bin
if [ $? -ne 0 ]; then
	echo "copy libUIlib.dll failed."
	exit 4
fi

# 删除build目录
rm -rf build
