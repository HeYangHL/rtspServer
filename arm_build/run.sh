#!/bin/bash
cmake ../ -DCMAKE_INSTALL_PREFIX=../arm_install
make -j16
make install
