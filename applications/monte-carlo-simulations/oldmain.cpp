/*
 Code adapted from https://cameron-mcelfresh.medium.com/monte-carlo-integration-313b37157852
 Initial compilation g++ -o main main.cpp
  ./main
*/

#include <iostream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <future>
#include <mutex>
#include <thread>
#include "flexi.h"

#include <nlohmann/json.hpp>

using json = nlohmann::json;

double myFunction(double x);
void monteCarloEstimateSTD(std::mutex& m, std::vector<std::string>& results, double lowBound, double upBound, int iterations);

std::string function_name = "montecarlo-dev";

int main()
{

	double lowerBound, upperBound;
	int iterations;

	lowerBound = 1;
	upperBound = 5;


    std::mutex m;
    std::vector<std::string> results;
    std::vector<std::future<void>> workers;

	for(int i =1; i < 6; i++) {
		iterations = 2*pow(4,i);

        auto worker = std::async(monteCarloEstimateSTD, std::ref(m), std::ref(results), lowerBound, upperBound, iterations);
        workers.push_back(std::move(worker));
	}

    for (auto& worker : workers) {
        worker.get();
    }

    std::cout << "Printing all results: \n";
    for (auto& result : results) {
        std::cout << result << "\n";
    }

	return 0;
}

//Function to integrate
double myFunction(double x) {
	return pow(x,4)*exp(-x);
}

//Function to execute Monte Carlo integration on predefined function
void monteCarloEstimateSTD(std::mutex& m, std::vector<std::tuple<int, double, double>>& results;, const std::vector<std::tuple<int, int, double, double>>& payload) {

    #ifdef FLEXIUSE

    std::string payloadStr = "[";
    for (const auto& item : payload) {
        int id = std::get<0>(item);
        int iterations = std::get<1>(item);
        double lowBound = std::get<2>(item);
        double upBound = std::get<3>(item);

        payloadStr += "{\"id\": " + std::to_string(id) +
                      ", \"iterations\": " + std::to_string(iterations) +
                      ", \"lowBound\": " + std::to_string(lowBound) +
                      ", \"upBound\": " + std::to_string(upBound) + "},";
    }
    if (!payload.empty()) {
        payloadStr.pop_back(); // Remove the trailing comma
    }
    payloadStr += "]";
    std::string response = invoke(function_name, payload);

    // Parse the JSON string
    json jsonData = json::parse(response);
    // Extract the body and parse it as JSON
    std::string bodyString = jsonData["body"];
    json bodyData = json::parse(bodyString);

    // Extract the results and add them to a vector of tuples
    std::vector<std::tuple<int, double, double>> results;
    for (const auto& result : bodyData["results"]) {
        int id = result["id"];
        double estimate = result["estimate"];
        double std = result["std"];
        const std::lock_guard<std::mutex> lock(m);
        results.emplace_back(id, estimate, std);
    }


//        std::string payload = "{\"iterations\": " + std::to_string(iterations) +  ",  \"upBound\": " + std::to_string(upBound) + ", \"lowBound\": " + std::to_string(lowBound) + "}";
//        std::string response = invoke(function_name, payload);
//
//        std::cout << "Response: " << response << "\n";
//        // Parse the JSON string
//        json jsonData = json::parse(response);
//        // Extract the body and parse it as JSON
//        std::string bodyString = jsonData["body"];
//        json bodyData = json::parse(bodyString);
//        // Extract the statsArray and convert it to an array of doubles
//        std::vector<double> statsArray = bodyData["statsArray"].get<std::vector<double>>();
////        printf("Estimate for %.1f -> %.1f is %.3f, STD = %.4f, (%i iterations)\n",
////                    lowerBound, upperBound, statsArray[0], statsArray[1], iterations);
    #else

    for (const auto& item : payload) {
        int id = std::get<0>(item);
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
//
//        vector<double> statsArray[2]; //position 0 holds the estimate, position 1 holds the STD
//
//        double totalSum = 0;
//        double totalSumSquared = 0;
//
//        int iter = 0;
//
//        while (iter < iterations-1) {
//            double randNum = lowBound + (float(rand())/RAND_MAX) * (upBound-lowBound);
//
//            double functionVal = myFunction(randNum);
//
//            totalSum += functionVal;
//            totalSumSquared+= pow(functionVal,2);
//
//            iter++;
//        }
//
//        double estimate = (upBound-lowBound)*totalSum/iterations;
//        double expected = totalSum/iterations;
//
//        double expectedSquare = totalSumSquared/iterations;
//
//        double std = (upBound-lowBound)*pow( (expectedSquare-pow(expected,2))/(iterations-1) ,0.5);
//
//        statsArray[0] = estimate;
//        statsArray[1] = std;

//        printf("Estimate for %.1f -> %.1f is %.3f, STD = %.4f, (%i iterations)\n",
//        lowBound, upBound, statsArray[0], statsArray[1], iterations);

    #endif

    std::string result = "Estimate for " + std::to_string(lowBound) + " -> " + std::to_string(upBound) +
                                     " is " + std::to_string(statsArray[0]) + ", STD = " + std::to_string(statsArray[1]) +
                                     ", (" + std::to_string(iterations) + " iterations)\n";

    const std::lock_guard<std::mutex> lock(m);
    results.emplace_back(result);
}
