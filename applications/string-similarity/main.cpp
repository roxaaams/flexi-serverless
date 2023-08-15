#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <thread>
#include <mutex>
#include "flexi.h"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::string functionName = "stringsimilarity-dev";

struct LevenshteinResult {
    std::string str1;
    std::string str2;
    int distance;
};

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

void calculateLevenshtein(const std::string &str1, const std::string &str2, std::vector<LevenshteinResult> &results, std::mutex &mtx, int index) {
    int distance = -1;
   #ifdef FLEXIUSE
        std::string payload = "{\"str1\": \"" + str1 +"\", \"str2\": \"" + str2 + "\"}";
           std::string response = invoke(functionName, payload, index);
           // Parse the JSON string
           json jsonData = json::parse(response);
           // Extract the body and parse it as JSON
           std::string bodyString = jsonData["body"];
           json bodyData = json::parse(bodyString);
           distance = bodyData["distance"].get<int>();
   #else
    distance = levenshteinDistance(str1, str2);
    #endif

    std::lock_guard<std::mutex> lock(mtx); // Lock the mutex
    results.push_back({str1, str2, distance});
}

int main() {
    std::vector<std::pair<std::string, std::string>> stringPairs = {
        {"kitten", "sitting"},
        {"flaw", "lawn"},
        {"abc", "def"},
        // Add more pairs here
    };

    std::vector<LevenshteinResult> results;
    std::mutex resultsMutex; // Mutex for protecting access to the results vector
    std::vector<std::thread> threads;

    int index = 0;
    for (const auto &pair : stringPairs) {

        const std::string &str1 = pair.first;
        const std::string &str2 = pair.second;

        threads.emplace_back(calculateLevenshtein, str1, str2, std::ref(results), std::ref(resultsMutex), index);
        index++;
    }

    for (auto &thread : threads) {
        thread.join();
    }

    // Print the results
    for (const auto &result : results) {
        std::cout << "Levenshtein distance between \"" << result.str1 << "\" and \"" << result.str2 << "\" is: " << result.distance << std::endl;
    }

    return 0;
}
