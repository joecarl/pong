#!/bin/bash




SRC_DIR=./src/client/updater
BUILD_DIR=./build/client
OBJ_DIR=./obj/updater

GLOBAL_PARAMS="-std=c++17 -Wall -Wno-misleading-indentation -fexceptions -g"

mkdir -p $OBJ_DIR > /dev/null 2>&1
mkdir -p $BUILD_DIR > /dev/null 2>&1

SCRIPTS=()
SCRIPTS+=(main)
SCRIPTS+=(../../utils)

for SCRIPT in ${SCRIPTS[@]}; do

	BASENAME=${SCRIPT##*/}
	SRC_FILE=$SRC_DIR/$SCRIPT.cpp
	OBJ_FILE=$OBJ_DIR/$BASENAME.o

	if [ "$SRC_FILE" -nt "$OBJ_FILE" ]; then
		echo "Compiling "$SCRIPT" -> "$BASENAME".o ..."
		g++ $GLOBAL_PARAMS -c $SRC_FILE -o $OBJ_FILE

		if [ $? -ne 0 ]; then
			echo "Compiling ERROR! Press any key to exit...";
			read -n 1
			exit 1
		fi
	fi
done

echo "Linking ..."

g++ -o $BUILD_DIR/updater.exe $OBJ_DIR/* -static-libstdc++ -static-libgcc -lcurl

if [ $? -ne 0 ]; then
	echo "Linking ERROR! Exiting...";
	read -n 1
	exit 1
else
	echo "Linking OK!"
fi

RESOURCES_DIR=$SRC_DIR/resources

if [ -d "$RESOURCES_DIR" ]; then
	echo "Copying resources..."
	cp -rv $RESOURCES_DIR $BUILD_DIR
fi

echo "Done!"
read -n 1