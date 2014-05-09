#!/bin/bash

# check the caller is root
if [ $HOME != "/root" ]; then
	echo "ERROR: please run the script by root!"
	exit 100
fi

# check the necessary command
NECESSARY_COMMANDS=(gcc make git apt-get)
for command in ${NECESSARY_COMMANDS[@]};
do
	type ${command} >/dev/null 2>&1 || { echo "ERROR: the ${command} can't be found."; exit 200; }
done

# set up the source codes pathname
SOURCE_PREFIX=$HOME
EXPAT_PATH=${SOURCE_PREFIX}/expat-2.1.0
YAML_PATH=${SOURCE_PREFIX}/yaml-0.1.5
ZLIB_PATH=${SOURCE_PREFIX}/zlib-1.2.8
EJABBERD_PATH=${SOURCE_PREFIX}/ejabberd-13.12

# check all the source package, then clear all the original source codes path
SOURCE_PATHS=("${EXPAT_PATH}" "${YAML_PATH}" "${ZLIB_PATH}" "${EJABBERD_PATH}")
PACKAGE_PATHS=("${EXPAT_PATH}.tar.gz" "${YAML_PATH}.tar.gz" "${ZLIB_PATH}.tar.gz" "${EJABBERD_PATH}.tgz")

for path in ${PACKAGE_PATHS[@]};
do
	if [ ! -f "${path}" ]; then
		echo "ERROR: ${path} does't exist!"
		exit 1
	fi
done

for path in ${SOURCE_PATHS[@]};
do
	if [ -d "${path}" ]; then
		echo "delete the directory: ${path}"
		rm -rf ${path}
	fi
done

# git clone some libraries
if [ -d deps ]; then
	echo "delete the directory: deps"
	rm -rf deps
fi
mkdir deps
cd deps
# 经测试使用https速度更快些
git clone https://github.com/processone/cache_tab.git p1_cache_tab || exit 2
git clone https://github.com/processone/tls.git p1_tls || exit 2
git clone https://github.com/processone/stringprep.git p1_stringprep || exit 2
git clone https://github.com/processone/xml.git p1_xml || exit 2
git clone https://github.com/processone/p1_yaml.git || exit 2
git clone https://github.com/rds13/xmlrpc.git || exit 2
git clone https://github.com/processone/zlib.git || exit 2
git clone https://github.com/basho/lager.git || exit 2
git clone https://github.com/processone/eiconv.git || exit 2

git clone https://github.com/DeadZen/goldrush.git || exit 2
cd goldrush
git checkout 0.1.6 || exit 2
cd ..

cd ..

apt-get install -y erlang
if [ $? -ne 0 ]; then
	echo "apt-get erlang failed."
	exit 2
fi

apt-get install -y libcurl4-openssl-dev
if [ $? -ne 0 ]; then
	echo "apt-get openssl failed."
	exit 2
fi


# start compiling
echo "compiling expat..."
tar zxf ${EXPAT_PATH}.tar.gz
cd ${EXPAT_PATH}
./configure
make;make install
if [ $? -ne 0 ]; then
	echo "failed."
	exit 2
fi

cd -
rm -rf ${EXPAT_PATH}

echo "compiling YAML..."
tar zxf ${YAML_PATH}.tar.gz
cd ${YAML_PATH}
./configure
make;make install
if [ $? -ne 0 ]; then
	echo "failed."
	exit 2
fi

cd -
rm -rf ${YAML_PATH}

echo "compiling zlib..."
tar zxf ${ZLIB_PATH}.tar.gz
cd ${ZLIB_PATH}
./configure
make;make install
if [ $? -ne 0 ]; then
	echo "failed."
	exit 2
fi

cd -
rm -rf ${ZLIB_PATH}

echo "compiling ejabberd..."
tar zxf ${EJABBERD_PATH}.tgz
cd ${EJABBERD_PATH}
./configure
if [ $? -ne 0 ]; then
	echo "configure failed."
	exit 2
fi

mv ../deps .

make;make install
if [ $? -ne 0 ]; then
	echo "make failed."
	exit 2
fi

cd -
rm -rf ${EJABBERD_PATH}