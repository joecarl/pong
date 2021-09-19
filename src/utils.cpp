
#include "utils.h"

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

using namespace std;

string GetWaitString(){

	static string pts;
	static int resize_timer = 0, pts_len = 0;
	
	resize_timer++;
	resize_timer = resize_timer % 30;
	
	if(resize_timer == 19){
		pts = "...";
		pts.resize(++pts_len % 4);
	}
	
	return pts;
}

std::string extract_pkg(std::string& raw){
	
	std::string pkg = "";

	auto pos = raw.find("\r\n\r\n");

	if(pos != std::string::npos){
		pkg = raw.substr(0, pos);
		raw.erase(0, pos + 4);
	}

	return pkg;

}

std::string file_get_contents(std::string filepath){

	std::ifstream ifs(filepath);
	std::string content;
	
	content.assign( 
		(std::istreambuf_iterator<char>(ifs)),
		(std::istreambuf_iterator<char>()) 
	);

	return content;

}

bool file_exists(const std::string& name) {

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



// trim from start (in place)
string ltrim(string s) {

	s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
		return !std::isspace(ch);
	}));

	return s;

}

// trim from end (in place)
string rtrim(string s) {

	s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
		return !std::isspace(ch);
	}).base(), s.end());

	return s;

}

// trim from both ends (in place)
string trim(string s) {

	s = ltrim(s);
	s = rtrim(s);

	return s;

}