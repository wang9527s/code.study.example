#!/bin/bash

# 设置构建环境变量
export PKG_CONFIG_PATH="/opt/homebrew/opt/gpgme/lib/pkgconfig:\
    /opt/homebrew/opt/libassuan/lib/pkgconfig:\
    /opt/homebrew/opt/libgpg-error/lib/pkgconfig:\
    /opt/homebrew/opt/libarchive/lib/pkgconfig:\
    /opt/homebrew/opt/readline/lib/pkgconfig:\
    /opt/homebrew/opt/lmdb/lib/pkgconfig:\
    /opt/homebrew/opt/popt/lib/pkgconfig:\
    /opt/homebrew/opt/jansson/lib/pkgconfig:\
    $PKG_CONFIG_PATH"

export LDFLAGS="-L/opt/homebrew/opt/gpgme/lib \
    -L/opt/homebrew/opt/libassuan/lib \
    -L/opt/homebrew/opt/libgpg-error/lib \
    -L/opt/homebrew/opt/libarchive/lib \
    -L/opt/homebrew/opt/readline/lib \
    -L/opt/homebrew/opt/lmdb/lib \
    -L/opt/homebrew/opt/popt/lib \
    -L/opt/homebrew/opt/jansson/lib \
    $LDFLAGS"

export CPPFLAGS="-I/opt/homebrew/opt/gpgme/include \
    -I/opt/homebrew/opt/libassuan/include \
    -I/opt/homebrew/opt/libgpg-error/include \
    -I/opt/homebrew/opt/libarchive/include \
    -I/opt/homebrew/opt/readline/include \
    -I/opt/homebrew/opt/lmdb/include \
    -I/opt/homebrew/opt/popt/include \
    -I/opt/homebrew/opt/jansson/include \
    $CPPFLAGS"

export PATH="/opt/homebrew/bin:$PATH"

set -e

function install_env () {
    brew install gpgme heimdal libassuan libgpg-error pkg-config jansson \
        readline libarchive

    brew install cpanminus
    sudo cpanm Parse::Yapp

	# ➜  tools git:(dev_wb) ✗ python3 --version
	# Python 3.9.6
	# 注意： python版本不同，可能会有问题	
	pip3 install markdown dnspython
}

function build_samba() {
    local version="$1"

    rm -rf tmp && mkdir -p tmp
    cp "samba-${version}.tar.gz" tmp
    cd tmp
    rm -rf "samba-${version}"
    tar -zxvf "samba-${version}.tar.gz"
    cd "samba-${version}"

    # MacOS ACL 支持有问题，禁用ACL支持
    ./configure --prefix="$(pwd)/../../build_install_${version}" --without-acl-support
    make -j$(sysctl -n hw.ncpu) # V=1
    make install
}

install_env
build_samba "4.15.13"
echo "success"
