#ifndef H_FLEXI
#define H_FLEXI

#include <string>

std::string getENV();

std::string exec(const char* command);

std::string invokeAWS(std::string& function_name,  std::string& payload);

std::string invoke(std::string& function_name, std::string& payload);

std::string invokeAWS(std::string& function_name,  std::string& payload);

#endif // H_FLEXI
