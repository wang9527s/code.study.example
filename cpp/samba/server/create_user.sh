#!/bin/bash

set -e

version="4.15.13"

cp ./smb.conf "./build_install_${version}/etc"
sleep 1
echo "add samba user"
sudo "./build_install_${version}/bin/smbpasswd" -a mi
sleep 1
sudo chown -R mi:staff "build_install_${version}"
