#ifndef UTILSH
#define UTILSH

#include <string>

std::string GetWaitString();

std::string file_get_contents(std::string filepath);

std::string extract_pkg(std::string& raw);

bool file_exists(const std::string& name);

std::string exec(const char* cmd);

// trim from both ends
std::string trim(std::string s);

#endif