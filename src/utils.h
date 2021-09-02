#ifndef UTILSH
#define UTILSH

#include <string>

std::string GetWaitString();

std::string file_get_contents(std::string filepath);

std::string extract_pkg(std::string& raw);

#endif