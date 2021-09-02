#ifndef NETUTILSH
#define NETUTILSH

#include <string>

std::string file_get_contents(std::string filepath);

std::string extract_pkg(std::string& raw);

#endif