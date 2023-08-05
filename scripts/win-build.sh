#!/bin/bash 
if [ "$1" == "test" ] || [ "$1" == "t" ]; then
	echo "[WIN BUILD] TESTING BUILD"
	/c/msys64/msys2_shell.cmd -ucrt64 scripts/build-test.sh win
elif [ "$1" == "gch" ] || [ "$1" == "h" ]; then
	echo "[WIN BUILD] PRECOMPILING HEADERS"
	/c/msys64/msys2_shell.cmd -ucrt64 scripts/build-gch.sh win
elif [ "$1" == "updater" ] || [ "$1" == "u" ]; then
	echo "[WIN BUILD] COMPILING UPDATER"
	/c/msys64/msys2_shell.cmd -ucrt64 scripts/build-updater.sh win
elif [ "$1" == "server" ] || [ "$1" == "s" ]; then
	echo "[WIN BUILD] COMPILING SERVER"
	/c/msys64/msys2_shell.cmd -ucrt64 scripts/build-server.sh win
else
	echo "[WIN BUILD] COMPILING CLIENT"
	/c/msys64/msys2_shell.cmd -ucrt64 scripts/build.sh win
fi
