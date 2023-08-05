#!/bin/bash




SRC_DIR=./src/server
BUILD_DIR=./build/server
OBJ_DIR=./obj/server

GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -include scripts/pch.hpp"

mkdir -p $BUILD_DIR > /dev/null 2>&1
mkdir -p $OBJ_DIR > /dev/null 2>&1

SCRIPTS=()
SCRIPTS+=(../ponggame)
SCRIPTS+=(../utils)
SCRIPTS+=(../udpcontroller)
SCRIPTS+=(clients)
SCRIPTS+=(group)
SCRIPTS+=(server)
SCRIPTS+=(main)


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

LINKERLIBS=""
TARGET_EXEC=""
if [ "$1" == "win" ]; then
	LINKERLIBS="-static-libstdc++ -static-libgcc -static \
				-lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lmswsock -lws2_32"
	TARGET_EXEC="pongserver.exe"
else 
	LINKERLIBS="-lboost_chrono -lboost_system -lboost_thread -lboost_json -lpthread"
	TARGET_EXEC="pongserver"
fi

g++ -o $BUILD_DIR/$TARGET_EXEC $OBJ_DIR/* $LINKERLIBS

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

if [ "$1" == "win" ]; then
	read -n 1
fi
