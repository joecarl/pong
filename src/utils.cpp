
#include "utils.h"
#include "appinfo.h"

#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <fstream>
#include <algorithm> 
#include <cctype>
#include <locale>
#include <filesystem>
#include <chrono>
#include <iomanip>

using namespace std;


#ifdef __ANDROID__
#include <android/log.h>
#endif

void log(const string& txt) {

#ifdef __ANDROID__

	__android_log_print(ANDROID_LOG_VERBOSE, APP_PKGNAME, "%s", txt.c_str());

#else
	
	cout << txt << endl;
	
#endif

}


const string get_storage_dir() {

#ifdef __ANDROID__

	return "/data/data/" APP_PKGNAME "/files";

#else

	string path = "./data"; // TODO: get real writable path

	filesystem::create_directories(path);

	return path;

#endif

}


string date() {

	time_t t = chrono::system_clock::to_time_t(chrono::system_clock::now());

	stringstream outbuff;	
	outbuff << put_time(localtime(&t), "%FT%T%z");
	return outbuff.str();

}


const string& get_wait_string() {

	static string pts;
	static uint8_t resize_timer = 0;
	static uint8_t pts_len = 0;
	
	resize_timer++;
	resize_timer = resize_timer % 30;
	
	if (resize_timer == 19) {
		pts = "...";
		pts.resize(++pts_len % 4);
	}
	
	return pts;
}


string extract_pkg(string& buffer) {
	
	string pkg = "";

	auto pos = buffer.find("\r\n\r\n");

	if (pos != string::npos) {
		pkg = buffer.substr(0, pos);
		buffer.erase(0, pos + 4);
	}

	return pkg;

}


string file_get_contents(const string& filepath) {

	ifstream ifs(filepath);
	string content;
	
	content.assign( 
		(istreambuf_iterator<char>(ifs)),
		(istreambuf_iterator<char>()) 
	);

	return content;

}


bool file_put_contents(const string& filepath, const string& contents) {

	ofstream file;
	file.open(filepath);
	file << contents;
	file.close();

	return file.good();
	
}


bool file_exists(const string& name) {

	if (FILE *file = fopen(name.c_str(), "r")) {
		fclose(file);
		return true;
	} else {
		return false;
	}

}


string exec(const char* cmd) {

	array<char, 128> buffer;
	string result;
	unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

	if (!pipe) {
		throw runtime_error("popen() failed!");
	}

	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
		result += buffer.data();
	}

	return result;

}


string ltrim(string s) {

	s.erase(s.begin(), find_if (s.begin(), s.end(), [] (unsigned char ch) {
		return !isspace(ch);
	}));

	return s;

}


string rtrim(string s) {

	s.erase(find_if(s.rbegin(), s.rend(), [] (unsigned char ch) {
		return !isspace(ch);
	}).base(), s.end());

	return s;

}


string trim(string s) {

	s = ltrim(s);
	s = rtrim(s);

	return s;

}