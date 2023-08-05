#!/bin/bash

GLOBAL_PARAMS="-Wall -fexceptions -g -x c++-header"

SRC_FILE=./scripts/pch.hpp

echo "Compiling "$SRC_FILE" ..."
g++ $GLOBAL_PARAMS -c $SRC_FILE

if [ $? -ne 0 ]; then
	echo "Compiling ERROR! Press any key to exit...";
	read -n 1
	exit 1
fi


echo "Done!"
if [ "$1" == "win" ]; then
	read -n 1
fi