#!/bin/bash

set -e

version="4.15.13"
smb_root="$(pwd)/build_install_${version}"
OS_TYPE="$(uname)"

if [ "$OS_TYPE" == "Darwin" ]; then
    echo "This is macOS"
    export DYLD_LIBRARY_PATH=$smb_root/lib:$DYLD_LIBRARY_PATH
    cp smb_mac.conf "$smb_root/etc/smb.conf"
    ./stop_sys_service.sh
elif [ "$OS_TYPE" == "Darwin" ]; then
    export LD_LIBRARY_PATH=$smb_root/lib:$LD_LIBRARY_PATH
    cp smb_linux.conf "$smb_root/etc/smb.conf"
fi

PIDS=$(pgrep smbd | tr '\n' ' ' || true)
if [ -n "$PIDS" ]; then
    echo "kill smbd $PIDS"
    kill -9 $PIDS 2>/dev/null || echo "kill smbd failed"
fi

cd build_install_${version}

echo "start smbd"
./sbin/smbd -d 3 -s ./etc/smb.conf
sleep 1
echo "new smbd pids: $(pgrep smbd | tr '\n' ' ' || true)"

