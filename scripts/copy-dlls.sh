BUILD_DIR=$1
cd $BUILD_DIR

# This function will list all required dlls and filter out the ones provided by the system
run_lld() {
	ldd ./*.exe | grep -iv system32 | grep -vi windows | grep -v :$ | cut -f2 -d\> | cut -f1 -d\( | tr '\\' '/'
}

# Run cmd and copy obtained files
run_lld | while read a; do 
	! [ -e "./$(basename $a)" ] && cp -v "$a" ./; 
done

exit 0
