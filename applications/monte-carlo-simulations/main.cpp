/*
 Code adapted from https://cameron-mcelfresh.medium.com/monte-carlo-integration-313b37157852
*/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <future>
#include <mutex>
#include <thread>
#include <tuple>
#include <chrono>
#include <nlohmann/json.hpp>
#include "flexi.h"

using json = nlohmann::json;

double myFunction(double x);
void monteCarloEstimateSTD(std::mutex& m, std::vector<std::tuple<std::string, double, double>>& results, const std::vector<std::tuple<std::string, int, double, double>>& payload);

std::string function_name = "montecarlo-dev";
std::vector<std::vector<std::tuple<std::string, int, double, double>>> payloads;

int main(int argc, char* argv[])
{
    std::mutex m;
    std::vector<std::future<void>> workers;
    std::vector<std::tuple<std::string, double, double>> results;

	double lowerBound = 100, upperBound = 5000;

	int threads = 2, totalInvocations = 4, iterations = 2;

    // Parse command-line arguments
    if (argc > 1) threads = std::stoi(argv[1]);
    if (argc > 2) totalInvocations = std::stoi(argv[2]);
    if (argc > 3) iterations = std::stoi(argv[3]);

    int invocationsPerThread = totalInvocations / threads;
    int remainingInvocations = totalInvocations % threads;

	for(int i = 0; i < totalInvocations; i++) {
         std::vector<std::tuple<std::string, int, double, double>> payload;
         payloads.emplace_back(payload);

		 for ( int j = 0; j < iterations; ++j) {
            payloads[i].emplace_back(std::to_string(i) + '_' + std::to_string(j), 5, lowerBound, upperBound);
         }

    }
//    for(int i = 0; i < totalInvocations; i++) {
//        auto worker = std::async(monteCarloEstimateSTD, std::ref(m), std::ref(results), std::ref(payloads[i]));
//        workers.push_back(std::move(worker));
//	}

    for(int t = 0; t < threads; t++) {
        int start = t * invocationsPerThread;
        int end = (t + 1) * invocationsPerThread;
        if (t == threads - 1) {
            end += remainingInvocations;
        }

        auto worker = std::async(std::launch::async, [&, start, end]() {
            for (int i = start; i < end; i++) {
                monteCarloEstimateSTD(m, results, payloads[i]);
            }
        });
        workers.push_back(std::move(worker));
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    for (auto& worker : workers) {
        worker.get();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
//    std::cout << "Elapsed time: " << duration << " milliseconds\n";

    #ifdef FLEXIUSE
    std::cout << "flexi-serverless " << threads << " " << totalInvocations << " " << iterations << " " << duration << "\n";
    #else
    std::cout << "local " << threads << " " << totalInvocations << " " << iterations << " " << duration << "\n";
    #endif

//    std::cout << "Printing all results: \n";
//     for (const auto& result : results) {
//            std::string id = std::get<0>(result);
//            double estimate = std::get<1>(result);
//            double std = std::get<2>(result);
//            std::cout << "ID: " << id << ", Estimate: " << estimate << ", STD: " << std << "\n";
//     }

	return 0;
}

//Function to integrate
double myFunction(double x) {
	return pow(x,4)*exp(-x);
}


//Function to execute Monte Carlo integration on predefined function
void monteCarloEstimateSTD(std::mutex& m, std::vector<std::tuple<std::string, double, double>>& results, const std::vector<std::tuple<std::string, int, double, double>>& payload) {

    #ifdef FLEXIUSE

//    std::cout << "1\n" ;

    std::string payloadStr = "[";
    if (payload.empty()) {
        throw std::invalid_argument("Payload cannot be empty");
    }
    for (const auto& item : payload) {
//        std::cout << "1.1\n" ;
        std::string id = std::get<0>(item);
//        std::cout << "1.2" << id << "\n" ;
        int iterations = std::get<1>(item);
//        std::cout << "1.3" << iterations<< "\n" ;
        double lowBound = std::get<2>(item);
//        std::cout << "1.4 " << lowBound << "\n" ;
        double upBound = std::get<3>(item);
//        std::cout << "1.5 " << upBound << "\n" ;
        payloadStr += "{\"id\": \"" + id +
                         "\", \"iterations\": " + std::to_string(iterations) +
                      ", \"lowBound\": " + std::to_string(lowBound) +
                      ", \"upBound\": " + std::to_string(upBound) + "},";
//                std::cout << "1.6\n" ;

    }
//            std::cout << "1.7\n" ;

    if (!payload.empty()) {
        payloadStr.pop_back(); // Remove the trailing comma
    }
//            std::cout << "1.8\n" ;

    payloadStr += "]";
//    std::cout << "2\n";
    std::string response = invoke(function_name, payloadStr);

    // Parse the JSON string
//    std::cout << "3\n";
//    std::cout << response << "\n";
    json jsonData = json::parse(response);
    // Extract the body and parse it as JSON
    std::string bodyString = jsonData["body"];
    json bodyData = json::parse(bodyString);

    // Extract the results and add them to a vector of tuples
    for (const auto& result : bodyData["results"]) {
        std::string id = result["id"];
        double estimate = result["estimate"];
        double std = result["std"];
        const std::lock_guard<std::mutex> lock(m);
        results.emplace_back(id, estimate, std);
    }

    #else

    for (const auto& item : payload) {
        std::string id = std::get<0>(item);
        int iterations = std::get<1>(item);
        double lowBound = std::get<2>(item);
        double upBound = std::get<3>(item);

        if (iterations <= 0 || lowBound >= upBound) {
            throw std::invalid_argument("Iterations must be positive and lowBound must be less than upBound");
        }

        double totalSum = 0;
        double totalSumSquared = 0;

        for (int iter = 0; iter < iterations; ++iter) {
            double randNum = lowBound + (static_cast<double>(rand()) / RAND_MAX) * (upBound - lowBound);
            double functionVal = myFunction(randNum);

            totalSum += functionVal;
            totalSumSquared += pow(functionVal, 2);
        }

        double estimate = (upBound - lowBound) * totalSum / iterations;
        double expected = totalSum / iterations;
        double expectedSquare = totalSumSquared / iterations;
        double std = (upBound - lowBound) * sqrt((expectedSquare - pow(expected, 2)) / (iterations - 1));

        const std::lock_guard<std::mutex> lock(m);
        results.emplace_back(id, estimate, std);
    }

    #endif
}
