#!/bin/bash




SRC_DIR=./src
BUILD_DIR=./build

GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -C -I$SRC_DIR -IC:/msys64/mingw64/include/freetype2"

mkdir ./build > /dev/null 2>&1
mkdir $BUILD_DIR > /dev/null 2>&1
mkdir $BUILD_DIR/obj > /dev/null 2>&1

scripts=()
scripts+=(hengine)
scripts+=(stages)
scripts+=(classes)
scripts+=(utils)
scripts+=(ioclient)
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

ADDL_LINK_OPTS=""

if [ $1 == "win" ]; then
	echo "Compiling Windows resources ..."
	for RCFILE in $SRC_DIR/windres/*.rc; do 
		bname=$(basename $RCFILE);
		echo "$RCFILE ---> $bname.res";
		windres $RCFILE -O coff -o $BUILD_DIR/$bname.res;
	done
	ADDL_LINK_OPTS=$BUILD_DIR/*.res
fi


echo "Linking ..."

g++ -o $BUILD_DIR/PONG.exe $BUILD_DIR/obj/* -static-libstdc++ -static-libgcc \
-lallegro -lallegro_image -lallegro_main -lallegro_ttf -lallegro_primitives -lallegro_audio -lallegro_font \
-lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lws2_32 \
$ADDL_LINK_OPTS

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