#!/bin/bash
# 构建Psychokinesis用于通信的服务器
# 目前只支持debian类的Linux系统
# 软件列表：
# 即时通讯服务器：ejabberd
# 支持BOSH和WebSocket：node-xmpp-bosh

. ./common.sh

SERVER_SOURCE_PATH=../src/server

# check the necessary command
NECESSARY_COMMANDS=(apt-get sudo)
for command in ${NECESSARY_COMMANDS[@]};
do
	type ${command} >/dev/null 2>&1 || { red_echo "ERROR: the ${command} can't be found."; exit 200; }
done


# 添加erlang最新源
grep -q erlang-solutions /etc/apt/sources.list
if [ $? -ne 0 ]; then
	cat /etc/issue | grep -q "Debian GNU/Linux 7"
	if [ $? -ne 0 ]; then
		sudo echo "deb http://packages.erlang-solutions.com/debian squeeze contrib" >> /etc/apt/sources.list
	else
		sudo echo "deb http://packages.erlang-solutions.com/debian wheezy contrib" >> /etc/apt/sources.list
	fi
	
	wget http://packages.erlang-solutions.com/debian/erlang_solutions.asc
	sudo apt-key add erlang_solutions.asc
	rm -f erlang_solutions.asc
	sudo apt-get update
fi

sudo apt-get install -y --force-yes erlang
if [ $? -ne 0 ]; then
	red_echo "apt-get erlang failed."
	exit 2
fi


# 需要安装最新版本的ejabberd
sudo apt-get -t unstable install -y --force-yes ejabberd ejabberd-contrib
if [ $? -ne 0 ]; then
	red_echo "apt-get ejabberd for unstable failed."
	exit 2
fi

# 编译安装Psychokinesis的ejabberd插件
cd ${SERVER_SOURCE_PATH}/mod_admin_extra
sudo ./build.sh
if [ $? -ne 0 ]; then
	red_echo "build mod_admin_extra failed."
	exit 3
fi
cd ..

cd ${SERVER_SOURCE_PATH}/mod_filter
sudo ./build.sh
if [ $? -ne 0 ]; then
	red_echo "build mod_filter failed."
	exit 3
fi
cd ..

cd ${SERVER_SOURCE_PATH}/mod_restful
sudo make install
if [ $? -ne 0 ]; then
	red_echo "build mod_restful failed."
	exit 3
fi
cd ..

cd ${SERVER_SOURCE_PATH}/mod_add_myself
sudo ./build.sh
if [ $? -ne 0 ]; then
	red_echo "build mod_add_myself failed."
	exit 3
fi
cd ..


# 安装nodejs
type node >/dev/null 2>&1
if [ $? -ne 0 ]; then
	install_node apt-get
	if [ $? -ne 0 ]; then
		red_echo "build nodejs failed."
		exit 4
	fi
fi

# 安装node-xmpp-bosh
sudo npm -g install node-xmpp-bosh
if [ $? -ne 0 ]; then
	red_echo "build node-xmpp-bosh failed."
	exit 4
fi

red_echo "Finish!"
