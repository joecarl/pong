
#include "utils.h"

#include <fstream>

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