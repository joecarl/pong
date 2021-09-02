
#include "netutils.h"

std::string extract_pkg(std::string& raw){
	
	std::string pkg = "";

	auto pos = raw.find("\r\n\r\n");

	if(pos != std::string::npos){
		pkg = raw.substr(0, pos);
		raw.erase(0, pos + 4);
	}

	return pkg;

}