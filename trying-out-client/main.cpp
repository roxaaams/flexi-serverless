#include <cstdio>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include <thread>
#include <future>
#include <vector>
#include <mutex>

/*
Exec function taken from https://stackoverflow.com/a/478960
*/
std::string Exec(const char* command) {
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

void InvokeAWS(std::string& function_name, std::mutex& m, std::vector<std::string>& results) {
    std::string command = "aws lambda invoke --payload '{}' --function-name " + function_name + " --cli-binary-format raw-in-base64-out response.json >nul 2>nul";
    system(command.c_str());
    std::string result = Exec("cat response.json");
    const std::lock_guard<std::mutex> lock(m);
    results.emplace_back(result);
}

void HandlerExampleDev(std::mutex& m, std::vector<std::string>& results) {
    const std::lock_guard<std::mutex> lock(m);
    results.emplace_back("{\"statusCode\":200,\"body\":\"Hello from Lambda local!\"}");
}

void InvokeLocal(std::string& function_name, std::mutex& m, std::vector<std::string>& results) {
    if (function_name.compare("example-dev") == 0) {
        HandlerExampleDev(m, results);
    } else {
        std::cout << "Function not supported!" << std::endl;
        exit(-1);
    }
}

int main(int argc, char **argv) {
    int num_requests { 2 };
    std::string function_name {""}, provider {"local"};

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [--requests <number> e.g 2] [--function <name> e.g. example-dev] [--provider <name> e.g aws] [-local means local execution, default is remote]" << std::endl;
        exit(0);
    }

    for (int i = 0; i < argc; ++i) {
        if (std::string(argv[i]).compare("--requests") == 0 ) {
            num_requests = atoi(argv[++i]);
            if (num_requests < 1) {
                std::cout << "The number of requests has to be at least 1!" << std::endl;
                exit(-1);
            }
        } else if (std::string(argv[i]).compare("--function") == 0 ) {
            function_name = argv[++i];
            // TODO: Check if function exists
            std::cout << function_name << "\n";
        } else if (std::string(argv[i]).compare("--provider") == 0 ) {
            provider = argv[++i];
             // TODO: Check if provider exists
            std::cout << provider << "\n";
        } else if (std::string(argv[i]).compare("--help") == 0 ) {
            std::cout << "Usage: " << argv[0] << " [--requests <number> e.g 2] [--function <name> e.g. example-dev] [--provider <name> e.g aws] [-local means local execution, default is remote]" << std::endl;
            exit(0);
        }
    }

    if (function_name.empty()) {
        std::cout << "Please provide a function name!" << std::endl;
        exit(-1);
    }

    if (provider.empty()) {
        std::cout << "Please provide a provider name!" << std::endl;
        exit(-1);
    }

    std::mutex m;
    std::vector<std::string> results;
    std::vector<std::future<void>> workers;

   std::cout << "Invoking " << num_requests << " requests to " << function_name << " using " << provider << " provider" << std::endl;
   for (int it {0}; it < num_requests; ++it) {
       if (provider.compare("aws") == 0) {
            auto worker = std::async(InvokeAWS, std::ref(function_name), std::ref(m), std::ref(results));
            workers.push_back(std::move(worker));
       } else if (provider.compare("local") == 0) {
            auto worker = std::async(InvokeLocal, std::ref(function_name), std::ref(m), std::ref(results));
            workers.push_back(std::move(worker));
       } else {
            std::cout << "Provider not supported!" << std::endl;
            exit(-1);
       }
   }

   for (auto& worker : workers) {
       worker.get();
   }

   std::cout << "Printing all results: \n";
   for (const auto& res : results) {
      std::cout << res << "\n";
   }

   return 0;
}
