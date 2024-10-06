#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include "flexi.h"
#include <chrono>
#include <fstream>
#include <future>
#include <regex>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

using StringPair = std::pair<std::string, std::string>;
using StringPairVector = std::vector<StringPair>;
using StringPairIterator = StringPairVector::iterator;
using LevenshteinResult = std::tuple<int, int>;

std::string functionName = "stringsimilarity-dev";
StringPairVector stringPairs;
std::vector<LevenshteinResult> results;


int levenshteinDistance(const std::string &str1, const std::string &str2);
void calculateMultipleLevenshteins(
    const int& t,
    const int& invocationsPerThread,
    const int& pairsPerInvocation,
    const int& remainingInvocations,
       const StringPairVector &stringPairs,
        std::vector<LevenshteinResult> &results,
    std::mutex &mtx);
void calculateLevenshtein(
    const int startIdx,
    const int endIdx,
    const StringPairVector &stringPairs,
    std::vector<LevenshteinResult> &results,
    std::mutex &mtx);
 void readStringPairsFromFile(const std::string& filename);

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <filename> [threads] [pairs_per_invocation]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    int threads = (argc > 2) ? std::stoi(argv[2]) : 2;
    int totalInvocations = (argc > 3) ? std::stoi(argv[3]) : 4;
    int pairsPerInvocation = (argc > 4) ? std::stoi(argv[4]) : 1;

    readStringPairsFromFile(filename);

    std::mutex resultsMutex; // Mutex for protecting access to the results vector
    std::vector<std::future<void>> workers;

    int invocationsPerThread = totalInvocations / threads;
    int remainingInvocations = totalInvocations % threads;

    for (int t = 0; t < threads; ++t) {
//        for (int i = 0; i < invocationsForThisThread; ++i) {
//            int startIdx = (t * invocationsPerThread + std::min(t, remainingInvocations) + i) * pairsPerInvocation;
//            int endIdx = std::min(startIdx + pairsPerInvocation, static_cast<int>(stringPairs.size()));
//            if (startIdx >= endIdx) break;
//
//            workers.push_back(std::async(std::launch::async, calculateLevenshtein, startIdx, endIdx, std::ref(stringPairs), std::ref(results), std::ref(resultsMutex)));
//        }

            workers.push_back(std::async(std::launch::async,
                calculateMultipleLevenshteins,
                std::ref(t),
                std::ref(invocationsPerThread),
                std::ref(pairsPerInvocation),
                std::ref(remainingInvocations),
                std::ref(stringPairs),
                std::ref(results),
                std::ref(resultsMutex)));
    }

    auto startTime = std::chrono::high_resolution_clock::now();
    for (auto &worker : workers) {
        worker.get();
    }
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
     #ifdef FLEXIUSE
    std::cout << "flexi-serverless " << threads << " " << totalInvocations << " " << pairsPerInvocation << " " << duration << "\n";
    #else
    std::cout << "local " << threads << " " << totalInvocations << " " << pairsPerInvocation << " " << duration << "\n";
    #endif

//    // Print the results
//    for (const auto &result : results) {
//        std::cout << "Levenshtein distance for id \"" << std::get<0>(result) << "\" is \"" << std::get<1>(result) << std::endl;
//    }



    return 0;
}

void readStringPairsFromFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string str1, str2;
        if (iss >> str1 >> str2) {
            stringPairs.emplace_back(str1, str2);
        }
    }

    file.close();
}

int levenshteinDistance(const std::string &str1, const std::string &str2) {
    int len1 = str1.length();
    int len2 = str2.length();

    std::vector<std::vector<int>> dp(len1 + 1, std::vector<int>(len2 + 1, 0));

    for (int i = 0; i <= len1; ++i) {
        dp[i][0] = i;
    }
    for (int j = 0; j <= len2; ++j) {
        dp[0][j] = j;
    }

    for (int i = 1; i <= len1; ++i) {
        for (int j = 1; j <= len2; ++j) {
            int cost = (str1[i - 1] == str2[j - 1]) ? 0 : 1;
            dp[i][j] = std::min({dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + cost});
        }
    }

    return dp[len1][len2];
}


void calculateMultipleLevenshteins(
    const int& t,
    const int& invocationsPerThread,
    const int& pairsPerInvocation,
    const int&  remainingInvocations,
       const StringPairVector &stringPairs,
        std::vector<LevenshteinResult> &results,
    std::mutex &mtx) {
    int invocationsForThisThread = invocationsPerThread + (t < remainingInvocations ? 1 : 0);
    for (int i = 0; i < invocationsForThisThread; ++i) {
        int startIdx = (t * invocationsPerThread + std::min(t, remainingInvocations) + i) * pairsPerInvocation;
        int endIdx = std::min(startIdx + pairsPerInvocation, static_cast<int>(stringPairs.size()));
        if (startIdx >= endIdx) return;
        calculateLevenshtein(startIdx, endIdx, stringPairs, results, mtx);
    }

}

void calculateLevenshtein(
    const int startIdx,
    const int endIdx,
    const StringPairVector &stringPairs,
    std::vector<LevenshteinResult> &results,
    std::mutex &mtx) {

    int distance = -1;
    #ifdef FLEXIUSE
    std::string payload = "[";
    for (int it = startIdx; it  <= endIdx; ++it) {
        payload += "{\"id\":" + std::to_string(it) + ",\"str1\":\"" + stringPairs[it].first + "\",\"str2\":\"" + stringPairs[it].second + "\"},";
    }
    if (!payload.empty()) {
        payload.pop_back(); // Remove the trailing comma
    }
    payload += "]";
    std::string response = invoke(functionName, payload, startIdx);
    json jsonData = json::parse(response);
    // Extract the body and parse it as JSON
    std::string bodyString = jsonData["body"];
    json bodyData = json::parse(bodyString);

    // Extract the results and add them to a vector of tuples
    for (const auto& result : bodyData["results"]) {
        int id = result["id"];
        int distance = result["distance"];
        const std::lock_guard<std::mutex> lock(mtx);
        results.emplace_back(id, distance);
    }
    #else
    for (int it = startIdx; it <= endIdx; ++it) {
        int distance = levenshteinDistance(stringPairs[it].first, stringPairs[it].second);
        std::lock_guard<std::mutex> lock(mtx); // Lock the mutex
        results.emplace_back(it, distance);
    }
    #endif
}
