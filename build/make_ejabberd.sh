#!/bin/bash
# 编译ejabberd，目前只支持debian类的Linux系统

function red_echo()
{
	echo -e "\033[0;31;1m${1}\033[0m"
}

# check the caller is root
if [ $HOME != "/root" ]; then
	red_echo "ERROR: please run the script by root!"
	exit 100
fi

# check the necessary command
NECESSARY_COMMANDS=(apt-get gcc make git)
for command in ${NECESSARY_COMMANDS[@]};
do
	type ${command} >/dev/null 2>&1 || { red_echo "ERROR: the ${command} can't be found."; exit 200; }
done

# set up the source codes pathname
SOURCE_PREFIX=`pwd`
EXPAT_PATH=${SOURCE_PREFIX}/expat-2.1.0
YAML_PATH=${SOURCE_PREFIX}/yaml-0.1.5
EJABBERD_PATH=${SOURCE_PREFIX}/ejabberd-14.05

# check all the source package, then clear all the original source codes path
SOURCE_PATHS=("${EXPAT_PATH}" "${YAML_PATH}" "${EJABBERD_PATH}")
PACKAGE_PATHS=("${EXPAT_PATH}.tar.gz" "${YAML_PATH}.tar.gz" "${EJABBERD_PATH}.tgz")

for path in ${PACKAGE_PATHS[@]};
do
	if [ ! -f "${path}" ]; then
		red_echo "ERROR: ${path} does't exist!"
		exit 1
	fi
done

for path in ${SOURCE_PATHS[@]};
do
	if [ -d "${path}" ]; then
		red_echo "delete the directory: ${path}"
		rm -rf ${path}
	fi
done

# git clone some libraries
# 提前下载一些依赖库，经测试国内使用https速度更快些
# if [ -d deps ]; then
# 	echo "delete the directory: deps"
# 	rm -rf deps
# fi
# mkdir deps
# cd deps
# git clone https://github.com/processone/cache_tab.git p1_cache_tab || exit 2
# git clone https://github.com/processone/tls.git p1_tls || exit 2
# git clone https://github.com/processone/stringprep.git p1_stringprep || exit 2
# git clone https://github.com/processone/xml.git p1_xml || exit 2
# git clone https://github.com/processone/p1_yaml.git || exit 2
# git clone https://github.com/rds13/xmlrpc.git || exit 2
# git clone https://github.com/processone/zlib.git || exit 2
# git clone https://github.com/basho/lager.git || exit 2
# git clone https://github.com/processone/eiconv.git || exit 2

# git clone https://github.com/DeadZen/goldrush.git || exit 2
# cd goldrush
# git checkout 0.1.6 || exit 2
# cd ..

# cd ..

# 添加erlang最新源
grep -q erlang-solutions /etc/apt/sources.list
if [ $? -ne 0 ]; then
	cat /etc/issue | grep -q "Debian GNU/Linux 7"
	if [ $? -ne 0 ]; then
		echo "deb http://packages.erlang-solutions.com/debian squeeze contrib" >> /etc/apt/sources.list
	else
		echo "deb http://packages.erlang-solutions.com/debian wheezy contrib" >> /etc/apt/sources.list
	fi
	
	wget http://packages.erlang-solutions.com/debian/erlang_solutions.asc
	apt-key add erlang_solutions.asc
	rm -f add erlang_solutions.asc
	apt-get update
fi

apt-get install -y --force-yes erlang
if [ $? -ne 0 ]; then
	red_echo "apt-get erlang failed."
	exit 2
fi

apt-get install -y --force-yes libcurl4-openssl-dev
if [ $? -ne 0 ]; then
	red_echo "apt-get openssl failed."
	exit 2
fi


# start compiling
red_echo "compiling expat..."
tar zxf ${EXPAT_PATH}.tar.gz
cd ${EXPAT_PATH}
./configure
make;make install
if [ $? -ne 0 ]; then
	red_echo "compile expat failed."
	exit 2
fi

cd -
rm -rf ${EXPAT_PATH}

red_echo "compiling YAML..."
tar zxf ${YAML_PATH}.tar.gz
cd ${YAML_PATH}
./configure
make;make install
if [ $? -ne 0 ]; then
	red_echo "compile YAML failed."
	exit 2
fi

cd -
rm -rf ${YAML_PATH}

red_echo "compiling ejabberd..."
tar zxf ${EJABBERD_PATH}.tgz
cd ${EJABBERD_PATH}
./configure
if [ $? -ne 0 ]; then
	red_echo "configure ejabberd failed."
	exit 2
fi

# 将提前下好的库移到指定目录
if [ -d ../deps ]; then
	mv ../deps .
fi

make;make install
if [ $? -ne 0 ]; then
	red_echo "make ejabberd failed."
	exit 2
fi

cd -
rm -rf ${EJABBERD_PATH}

# 避免运行时可能出现找不到依赖库的情况
ldconfig

red_echo "Finish!"