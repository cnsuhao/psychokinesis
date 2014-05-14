#!/bin/sh

rm -rf ebin
mkdir ebin

erl -pa /lib/ejabberd/ebin -pz ebin -make || exit 1

# gen_mod.beam仅是中间文件
rm -f ebin/gen_mod.beam
