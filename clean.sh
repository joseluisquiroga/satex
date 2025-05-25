#!/bin/bash

CURR_DIR=$(pwd)
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

echo "clean ${1} using "${BASH_SOURCE[0]}

cd $SCRIPT_DIR

cd build
make clean

cd $SCRIPT_DIR

find . -mount -name "Makefile" -exec rm -f '{}' \;
find . -mount -name "*.log" -exec rm -f '{}' \;
find . -mount -name "*~" -exec rm -f '{}' \;

echo "Finished clean ----------------------------- "

cd ${CURR_DIR}