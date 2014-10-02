#!/bin/sh

EJABBERD_PATH=/usr/lib/x86_64-linux-gnu/ejabberd

if [ -d ebin ]; then
	rm -rf ebin
fi

mkdir ebin

erl -pa ${EJABBERD_PATH}/ebin -pz ebin -make || exit 1

cp -f ebin/* ${EJABBERD_PATH}/ebin
