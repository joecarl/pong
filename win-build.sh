#!/bin/bash 
if [ "$1" == "updater" ] || [ "$1" == "u" ]; then
	echo "[WIN BUILD] COMPILING UPDATER"
	/c/msys64/msys2_shell.cmd -mingw64 build-updater.sh win
elif [ "$1" == "server" ] || [ "$1" == "s" ]; then
	echo "[WIN BUILD] COMPILING SERVER"
	/c/msys64/msys2_shell.cmd -mingw64 build-server.sh win
else
	echo "[WIN BUILD] COMPILING CLIENT"
	/c/msys64/msys2_shell.cmd -mingw64 build.sh win
fi

