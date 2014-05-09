# 此脚本用来构建编译整个Psychokinesis项目所需要的编译环境
# 基于debian-7.3.0-amd64系统，其他debian系操作系统也应该适用
#!/bin/bash

NECESSARY_COMMANDS=(gcc make git p7zip bzip2 ntpdate gcc-mingw32)

for command in ${NECESSARY_COMMANDS[@]};
do
	type ${command} >/dev/null 2>&1
	if [ $? -ne 0 ]; then
		aptitude install -y ${command}
		if [ $? -ne 0 ]; then
			echo "install ${command} failed."
			exit 1
		fi
	fi
done

# 设置系统时间（请确保时区正确，使用dpkg-reconfigure tzdata设置）
ntpdate ntp.ubuntu.com
if [ $? -ne 0 ]; then
	echo "update time failed."
	exit 2
fi

echo "finished."