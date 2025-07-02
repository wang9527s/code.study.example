#!/bin/bash


set -e

function install_env () {
    sudo apt install -y \
        build-essential pkg-config \
        gnutls-bin libgnutls28-dev \
        liblmdb-dev bison flex libgpgme-dev \
        libpopt-dev libbsd-dev\
        build-essential libacl1-dev libattr1-dev \
        libblkid-dev libgnutls28-dev libreadline-dev  \
        attr bind9utils docbook-xsl libcups2-dev acl xsltproc \
        libreadline-dev libjansson-dev \
        libarchive-dev \
        libacl1-dev libcups2-dev libldap2-dev \
        libpam0g-dev libglib2.0-dev libicu-dev \
        libtracker-sparql-3.0-dev libdbus-1-dev \
        python3-dev python3-markdown python3-dnspython

    sudo apt install -y cpanminus
    sudo cpanm Parse::Yapp
}

# install_env

version="4.19.9"

tar -zxvf "samba-${version}.tar.gz"
cd "samba-${version}"

./configure --prefix="$(pwd)/../build_install_${version}"
make -j$(nproc)
make install

echo "success"
