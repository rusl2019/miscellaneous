#!/usr/bin/env bash

# tar xzvf ncurses-6.5.tar.gz
# cd ncurses-6.5/
# ./configure --prefix=$HOME/another/ncurses
# make -j$(nproc)
# make install
# cd ../

tar xzvf libssh2-1.11.0.tar.gz
cd libssh2-1.11.0
mkdir build
cd build
cmake .. -DCMAKE_INSTALL_PREFIX=/home/ruslan/another/libssh2
make -j$(nproc)
make install
cd ../..
