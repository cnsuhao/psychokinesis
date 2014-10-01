#!/bin/bash
# 此脚本用来构建Psychokinesis服务器运行所需要的环境
# 基于Linux，请在安装有包管理器（apt/yum）的Linux系统上运行
# 软件列表：
# Web服务器（静态资源）：nginx
# Web服务器（动态资源）：nodejs
# 数据库：MongoDB
# 即时通讯服务器：ejabberd（由于添加插件需要重新编译，使用make_ejabberd.sh安装）

SOFTWARE=(mongodb-org npm nodejs nginx)

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


# 查找系统可用的包管理器
type aptitude >/dev/null 2>&1
if [ $? -eq 0 ]; then
	INSTALLER=aptitude
fi

type yum >/dev/null 2>&1
if [ $? -eq 0 ]; then
	INSTALLER=yum
fi

if [ "${INSTALLER}" = "" ]; then
	red_echo "No package installer found!"
	exit 1
fi

# 导入需要用到的一些库
if [ "${INSTALLER}" = "yum" ]; then
	# MongoDB
	if [ ! -f /etc/yum.repos.d/mongodb.repo ];then
		echo "[MongoDB]
		name=MongoDB Repository
		baseurl=http://downloads-distro.mongodb.org/repo/redhat/os/x86_64
		gpgcheck=0
		enabled=1" | sudo tee -a /etc/yum.repos.d/mongodb.repo
	fi
elif [ "${INSTALLER}" = "aptitude" ]; then
	# MongoDB
	if [ ! -f /etc/apt/sources.list.d/mongodb.list ];then
		sudo apt-key adv --keyserver keyserver.ubuntu.com --recv 7F0CEB10
		echo 'deb http://downloads-distro.mongodb.org/repo/debian-sysvinit dist 10gen' | sudo tee /etc/apt/sources.list.d/mongodb.list
	fi
fi

sudo ${INSTALLER} update -y

# 依次安装软件
for soft in ${SOFTWARE[@]};
do
	if [ "${INSTALLER}" = "aptitude" ]; then
		cat /etc/issue | grep -q "Debian"
		if [ $? -eq 0 ]; then
			# nodejs和npm需要执行添加源脚本后再安装
			if [ "${soft}" = "npm" ] || [ "${soft}" = "nodejs" ]; then
				INSTALL_NODEJS_FROM_SOURCE="yes"
				continue
			fi
		fi
		
		sudo ${INSTALLER} install -y ${soft}
		RET=$?
	elif [ "${INSTALLER}" = "yum" ]; then
		sudo ${INSTALLER} install -y ${soft}
		RET=$?
	fi
	
	if [ ${RET} -ne 0 ]; then
		red_echo "Install ${soft} failed."
		exit 2
	fi
done

if [ "${INSTALL_NODEJS_FROM_SOURCE}" = "yes" ]; then
	# 使用添加源的方式安装
	sudo ${INSTALLER} install -y curl
	if [ $? -ne 0 ]; then
		red_echo "Install curl failed. Npm's installation need it."
		exit 2
	fi
	
	RET=2
	if [ "${INSTALLER}" = "yum" ]; then
		curl -sL https://rpm.nodesource.com/setup | sudo bash -
		RET=$?
	elif [ "${INSTALLER}" = "aptitude" ]; then
		curl -sL https://deb.nodesource.com/setup | sudo bash -
		RET=$?
	fi
	
	if [ ${RET} -ne 0 ]; then
		red_echo "Add package source failed."
		exit 2
	fi
	
	sudo ${INSTALLER} install -y nodejs
	if [ $? -ne 0 ]; then
		red_echo "Install nodejs failed."
		exit 2
	fi
fi

red_echo "All the software that psychokinesis required has been installed."
