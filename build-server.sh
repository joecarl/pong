#!/bin/bash




SRC_DIR=./src/server
BUILD_DIR=./build/server

GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -C -I$SRC_DIR -IC:/msys64/mingw64/include/freetype2"

mkdir ./build > /dev/null 2>&1
mkdir $BUILD_DIR > /dev/null 2>&1
mkdir $BUILD_DIR/obj > /dev/null 2>&1

scripts=()
scripts+=(/../classes)
scripts+=(/../netutils)
scripts+=(clients)
scripts+=(group)
scripts+=(server)
scripts+=(main)


for SCRIPT in ${scripts[@]}; do

	basename=${SCRIPT##*/}
	source_file=$SRC_DIR/$SCRIPT.cpp
	target_file=$BUILD_DIR/obj/$basename.o

	if [ "$source_file" -nt "$target_file" ]; then
		echo "Compiling "$SCRIPT" -> "$basename".o ..."
		g++ $GLOBAL_PARAMS -c $source_file -o $target_file

		if [ $? -ne 0 ]; then
			echo "Compiling ERROR! Press any key to exit...";
			read -n 1
			exit 1
		fi
	fi
done

echo "Linking ..."

g++ -o $BUILD_DIR/PONGSERVER.exe $BUILD_DIR/obj/* -static-libstdc++ -static-libgcc -static \
-lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lws2_32 \

if [ $? -ne 0 ]; then
	echo "Linking ERROR! Exiting...";
	read -n 1
	exit 1
else
	echo "Linking OK!"
fi

resources_dir=$SRC_DIR/resources

if [ -d "$resources_dir" ]; then
	echo "Copying resources..."
	cp -rv $resources_dir $BUILD_DIR
fi

echo "Done!"
read -n 1