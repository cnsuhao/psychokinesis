#!/bin/bash
# 一些公共的函数

function red_echo()
{
	echo -e "\033[0;31;1m${1}\033[0m"
}

function read_string()
{
    while :
    do
        read -p "$1"
        if [ "${REPLY}" != "" ]; then
            break
        fi
    done

    echo ${REPLY}
}

function path_available()
{
	sudo find $1 -type f -exec chmod 644 {} \;
	sudo find $1 -type d -exec chmod 755 {} \;
}

function install_node()
{
	if [ "${1}" = "" ]; then
		red_echo "No package installer found!"
		return 1
	fi
	
	type sudo >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		red_echo "sudo can't be found!"
		return 1
	fi
	
	# 使用添加源的方式安装
	sudo ${1} install -y curl
	if [ $? -ne 0 ]; then
		red_echo "Install curl failed. Npm's installation need it."
		return 2
	fi
	
	RET=2
	if [ "${1}" = "yum" ]; then
		curl -sL https://rpm.nodesource.com/setup | sudo bash -
		RET=$?
	elif [ "${1}" = "aptitude" ]; then
		curl -sL https://deb.nodesource.com/setup | sudo bash -
		RET=$?
	fi
	
	if [ ${RET} -ne 0 ]; then
		red_echo "Add package source failed."
		return 2
	fi
	
	sudo ${1} install -y nodejs
	if [ $? -ne 0 ]; then
		red_echo "Install nodejs failed."
		return 2
	fi
	
	return 0
}
