#!/bin/bash




SRC_DIR=./src/client
BUILD_DIR=./build/client
OBJ_DIR=./obj/client

GLOBAL_PARAMS="-Wall -Wno-misleading-indentation -fexceptions -g -C -I$SRC_DIR -IC:/msys64/ucrt64/include/freetype2"

mkdir -p $BUILD_DIR > /dev/null 2>&1
mkdir -p $OBJ_DIR > /dev/null 2>&1

scripts=()
scripts+=(/../ponggame)
scripts+=(/../utils)
scripts+=(touchkeys)
scripts+=(hengine)
scripts+=(stages)
scripts+=(onlinestages)
scripts+=(mediatools)
scripts+=(ioclient)
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

#ADDL_LINK_OPTS=""

if [ "$1" == "win" ]; then
	echo "Compiling Windows resources ..."
	for RCFILE in $SRC_DIR/windres/*.rc; do 
		bname=$(basename $RCFILE);
		echo "$RCFILE ---> $bname.res";
		windres $RCFILE -O coff -o $OBJ_DIR/$bname.res;
	done
	#ADDL_LINK_OPTS=$BUILD_DIR/*.res
	ADDL_LIBS="-lboost_chrono-mt -lboost_system-mt -lboost_thread-mt -lboost_json-mt -lws2_32"
else 
	ADDL_LIBS="-lboost_chrono -lboost_system -lboost_thread -lboost_json -lpthread"
fi


echo "Linking ..."

g++ -o $BUILD_DIR/PONG.exe $OBJ_DIR/* -static-libstdc++ -static-libgcc \
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

resources_dir=$SRC_DIR/resources

if [ -d "$resources_dir" ]; then
	echo "Copying resources..."
	cp -rv $resources_dir $BUILD_DIR
fi

echo "Done!"
read -n 1