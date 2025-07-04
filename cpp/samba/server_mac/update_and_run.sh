#!/bin/bash

set -e

version="4.15.13"

function add_user() {
    sudo ./build_install_4.15.13/bin/smbpasswd -a mi
}

export DYLD_LIBRARY_PATH=/Users/mi/samba/build_install_${version}/lib:$DYLD_LIBRARY_PATH

if sudo launchctl list | grep -q "$com.apple.smbd"; then
    echo "stop sys smb"
    sudo launchctl bootout system /System/Library/LaunchDaemons/com.apple.smbd.plist
else
    echo "sys smb not start"
fi

PIDS=$(pgrep smbd || true)
if [ -n "$PIDS" ]; then
    sudo kill -9 $PIDS 2>/dev/null || echo "kill smbd failed"
fi
PIDS=$(pgrep nmbd || true)
if [ -n "$PIDS" ]; then
    sudo kill -9 $PIDS 2>/dev/null || echo "kill nmbd failed"
fi

sudo rm -f /Users/mi/samba/samba.log

cd build_install_${version}
cp ../smb.conf ./etc
sudo chown mi:staff ./etc/smb.conf

set -e

echo "start smbd"
sudo -E ./sbin/smbd -d 3 -s ./etc/smb.conf
sleep 2
echo "smbd: $(pgrep smbd)"

