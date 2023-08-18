#ifndef UTILS_H
#define UTILS_H

#include <string>

/**
 * Returns an ellipsis with 1, 2 o 3 periods. This function should be repeatedly called
 * so its effect is noticeable
 */
const std::string& get_wait_string();

/**
 * Reads entire file into a string 
 */
std::string file_get_contents(const std::string& filepath);

/**
 * Write data to a file
 */
bool file_put_contents(const std::string& filepath, const std::string& contents);

/**
 * Extracts a package form the provided buffer if the termination sequence is
 * found. Returns an empty string otherwise. The returned value is erased from 
 * the input variable
 */ 
std::string extract_pkg(std::string& buffer);

/**
 * Determines wether a file exists or not
 */ 
bool file_exists(const std::string& name);

/**
 * Run the specified command
 */ 
std::string exec(const char* cmd);

/**
 * Trim from start
 */ 
std::string ltrim(std::string s);

/**
 * Trim from end
 */
std::string rtrim(std::string s);

/**
 * Trim from both ends
 */ 
std::string trim(std::string s);

/**
 * Output data to the corresponding platform log system
 */ 
void log(const std::string& txt);

/**
 * Obtain a valid writeable directory to save data
 */ 
const std::string get_storage_dir();

#endif