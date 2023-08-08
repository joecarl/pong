#!/bin/bash




SRC_DIR=./src/client
BUILD_DIR=./build/client
OBJ_DIR=./obj/client

#GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -IC:/msys64/ucrt64/include/freetype2"
GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -include scripts/pch.hpp"

mkdir -p $BUILD_DIR > /dev/null 2>&1
mkdir -p $OBJ_DIR > /dev/null 2>&1

SCRIPTS=()
SCRIPTS+=(../ponggame)
SCRIPTS+=(../randomgenerator)
SCRIPTS+=(../utils)
SCRIPTS+=(../udpcontroller)
SCRIPTS+=(touchkeys)
SCRIPTS+=(hengine)
SCRIPTS+=(stages)
SCRIPTS+=(onlinestages)
SCRIPTS+=(mediatools)
SCRIPTS+=(ioclient)
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

#ADDL_LINK_OPTS=""

if [ "$1" == "win" ]; then
	echo "Compiling Windows resources ..."
	TARGET_EXEC=pong.exe
	for RCFILE in $SRC_DIR/windres/*.rc; do 
		BNAME=$(basename $RCFILE);
		echo "$RCFILE ---> $BNAME.res";
		windres $RCFILE -O coff -o $OBJ_DIR/$BNAME.res;
	done
	#ADDL_LINK_OPTS=$BUILD_DIR/*.res
	ADDL_LIBS="-lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lws2_32"
else 
	TARGET_EXEC=pong
	ADDL_LIBS="-lboost_chrono -lboost_system -lboost_thread -lboost_json -lpthread"
fi


echo "Linking ..."

g++ -o $BUILD_DIR/$TARGET_EXEC $OBJ_DIR/* -static-libstdc++ -static-libgcc \
-lallegro -lallegro_image -lallegro_main -lallegro_ttf -lallegro_primitives -lallegro_audio -lallegro_font \
$ADDL_LIBS
#$ADDL_LINK_OPTS

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
