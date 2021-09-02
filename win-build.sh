#!/bin/bash 

if [ "$1" == "server" ] || [ "$1" == "s" ]; then
    echo "[WIN BUILD] COMPILING SERVER"
	/c/msys64/msys2_shell.cmd -mingw64 build-server.sh win
else
    echo "[WIN BUILD] COMPILING CLIENT"
	/c/msys64/msys2_shell.cmd -mingw64 build.sh win
fi

