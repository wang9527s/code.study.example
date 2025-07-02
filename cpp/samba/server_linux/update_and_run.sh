#!/bin/bash

function add_user() {
    # 前提 wangbin用户存在，否则需要创建用户 sudo useradd -m wangbin
    sudo ./build_install_4.19.9/bin/smbpasswd -a wangbin
}

smb_root="$(pwd)/build_install_4.19.9"
export LD_LIBRARY_PATH=$smb_root/lib:$LD_LIBRARY_PATH

PIDS=$(pidof smbd)
if [ -n "$PIDS" ]; then
    echo "kill smbd: $PIDS"
    sudo kill -9 $PIDS 2>/dev/null || echo "kill smbd failed"
fi
PIDS=$(pidof nmbd)
if [ -n "$PIDS" ]; then
    echo "kill nmbd: $PIDS"
    sudo kill -9 $PIDS 2>/dev/null || echo "kill nmbd failed"
fi

cp -r smb.conf $smb_root/etc

set -e

sudo $smb_root/sbin/smbd -s  $smb_root/etc/smb.conf
sudo $smb_root/sbin/nmbd -s  $smb_root/etc/smb.conf

echo "smbd: $(pidof smbd)"
echo "nmbd: $(pidof nmbd)"