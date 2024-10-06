#include "flexi.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <mutex>
#include <array> // Include this header for std::array


std::string getENV() {
    const char* env_var_name = "FLEXI_PROVIDER";

    // Get the value of the environment variable
    const char* var_value = std::getenv(env_var_name);

    // Check if the environment variable exists
    if (var_value != nullptr) {
        std::string var_value_str(var_value);
        if (var_value_str == "AWS" || var_value_str == "GCP" || var_value_str == "AZURE") {
            return var_value_str;
        } else {
            std::cout << "Environment variable " << env_var_name << " is not set to a valid value." << std::endl;
            return "";
        }
    } else {
        std::cout << "Environment variable " << env_var_name << " not found." << std::endl;
        return "";
    }
}

/*
Exec function taken from https://stackoverflow.com/a/478960
*/
std::string exec(const char* command) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(command, "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }
    return result;
}

 std::string invokeAWS(const std::string& function_name, const std::string& payload, int index) {
    std::string command = "aws lambda invoke --payload '" + payload + "' --function-name " + function_name + " --cli-binary-format raw-in-base64-out responses/response" + std::to_string(index) + ".json >nul 2>nul";
//    std::cout << command << std::endl;
    system(command.c_str());
    command = "cat responses/response" + std::to_string(index) + ".json";
    std::string result = exec(command.c_str());
    return result;
}

std::string invokeGCP(const std::string& function_name, const std::string& payload) {
    std::string command = "gcloud functions call " + function_name + " --data '" + payload + "' >nul 2>nul";
//    std::cout << command << std::endl;
    system(command.c_str());
    std::string result = exec("cat response.json");
    return result;
}

 std::string invoke(const std::string& function_name, const std::string& payload, int index) {
    std::string provider = getENV();
    if (provider == "AWS") {
        return invokeAWS(function_name, payload, index);
    } else if (provider == "GCP") {
        return invokeGCP(function_name, payload);
    } else if (provider == "AZURE") {
//        return invokeAZURE(function_name, payload);
        return "Provider valid but not implemented yet";
    } else {
        std::cout << "Provider not found." << std::endl;
        return "Provider not found or invalid";
    }
 }
