#!/bin/bash
# 修改ssh配置，以允许/禁止root账号远程使用密码登录
# 建议在.profile或.bashrc中运行，示例：
# $HOME/change_root_ssh.sh 123 enable
# trap "$HOME/change_root_ssh.sh 123 disable" EXIT

function show_help()
{
	echo "usage: $0 current_user_password enable|disable"
}


if [ "$1" != "" ]; then
	MYPASSWORD=$1
else
	show_help
	exit 1
fi


if [ "$2" = "enable" ] || [ "$2" = "disable" ]; then
	SWITCH=$2
else
	show_help
	exit 1
fi

type sudo >/dev/null 2>&1
if [ $? -ne 0 ]; then
	echo "The command 'sudo' is missing!"
	exit 2
fi

SSH_CONFIG=/etc/ssh/sshd_config
if [ "${SWITCH}" = "enable" ]; then
	OPTION="yes"
else
	OPTION="no"
fi

echo $MYPASSWORD | sudo -S sed -i 's/PasswordAuthentication.*/PasswordAuthentication '${OPTION}'/' ${SSH_CONFIG} >> /dev/null
echo $MYPASSWORD | sudo -S sed -i 's/PermitRootLogin.*/PermitRootLogin '${OPTION}'/' ${SSH_CONFIG} >> /dev/null

echo $MYPASSWORD | sudo -S /etc/init.d/ssh restart >> /dev/null

if [ "${SWITCH}" = "enable" ]; then
	echo "OK! Now you can login by the root's password."
else
	echo "OK! The root and password are forbidden through the ssh."
fi
