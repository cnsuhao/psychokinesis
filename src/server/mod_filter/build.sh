#!/bin/sh

rm -rf ebin
mkdir ebin

erl -pa /lib/ejabberd/ebin -pz ebin -make || exit 1

# 以下仅是中间文件
rm -f ebin/lager_transform.beam
rm -f ebin/lager_util.beam
rm -f ebin/gen_mod.beam
