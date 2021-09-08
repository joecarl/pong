#!/bin/bash




SRC_DIR=./src/client/updater
BUILD_DIR=./build/client
OBJ_DIR=./obj/updater

GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -C -I$SRC_DIR "

mkdir -p $OBJ_DIR > /dev/null 2>&1
mkdir -p $BUILD_DIR > /dev/null 2>&1

scripts=()
scripts+=(main)

for SCRIPT in ${scripts[@]}; do

	basename=${SCRIPT##*/}
	source_file=$SRC_DIR/$SCRIPT.cpp
	target_file=$OBJ_DIR/$basename.o

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

g++ -o $BUILD_DIR/updater.exe $OBJ_DIR/* -static-libstdc++ -static-libgcc -lcurl

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