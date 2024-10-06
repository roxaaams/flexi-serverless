#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>
#include <future>


#include <nlohmann/json.hpp>

#include "base64.h"
#include "flexi.h"

using json = nlohmann::json;
using Result = std::tuple<int, int, int>;


std::string functionName = "imagefiltering-dev";
std::vector<Result> results;

void processImages(
    const std::string& imageFilenameOrURL,
     const int& imagesPerInvocation,
      std::vector<Result> &results,
      std::mutex &mtx) {
//      std::cout << "Processing images" << std::endl;
    #ifdef FLEXIUSE
    std::string payload = "{\"imagePairs\":[";
    for (int i = 0; i < imagesPerInvocation; ++i) {
        payload += "{\"id\":" + std::to_string(i) + ",\"imageUrl\":\"" + imageFilenameOrURL + "\"},";
    }
    if (!payload.empty()) {
        payload.pop_back(); // Remove the trailing comma
    }
    payload += "]}";
    std::string response = invoke(functionName, payload);
//    std::cout << "Response: " << response << std::endl;
    json jsonData = json::parse(response);
    // Extract the body and parse it as JSON
    std::string bodyString = jsonData["body"];
    json bodyData = json::parse(bodyString);

    // Extract the results and add them to a vector of tuples
    for (const auto& result : bodyData["results"]) {
        int id = result["id"];
        int blackPixels = result["blackPixels"];
        int whitePixels = result["whitePixels"];
        const std::lock_guard<std::mutex> lock(mtx);
        results.emplace_back(id, blackPixels, whitePixels);
    }
    #else
        for (int i = 0; i < imagesPerInvocation; ++i) {
            std::ifstream image;
            image.open(imageFilenameOrURL);

            // Copy over header information
            std::string type = "", width = "", height = "", RGB = "";
            image >> type >> width >> height >> RGB;

            std::string red = "", green = "", blue = "";
            int r = 0, g = 0, b = 0;
            int blackPixels = 0, whitePixels = 0;

            while (!image.eof()) {
                image >> red >> green >> blue;
                std::stringstream redStream(red);
                std::stringstream greenStream(green);
                std::stringstream blueStream(blue);

                redStream >> r;
                greenStream >> g;
                blueStream >> b;

                // Assuming that if R, G, and B components are equal, it's a grayscale pixel
               if (r == g && g == b) {
                   if (r == 0) {
                       blackPixels++;
                   } else if (r == 255) {
                       whitePixels++;
                   }
               }
             }
             image.close();
             const std::lock_guard<std::mutex> lock(mtx);
             results.emplace_back(i, blackPixels, whitePixels);
         }
    #endif
}

void processImagesPerThread(
    const int& t,
    const int& invocationsForThisThread,
    const int& imagesPerInvocation,
    const std::string& imageFilenameOrURL,
    std::vector<Result> &results,
    std::mutex &mtx) {
//    std::cout << "Processing images for thread " << t << std::endl;
    for (int i = 0; i < invocationsForThisThread; ++i) {
        processImages(imageFilenameOrURL, imagesPerInvocation, results, mtx);
    }
 
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << "  [threads] [totalInvocations] [imagesPerInvocation]" << std::endl;
        return 1;
    }

    int threads = (argc > 1) ? std::stoi(argv[1]) : 2;
    int totalInvocations = (argc > 2) ? std::stoi(argv[2]) : 4;
    int imagesPerInvocation = (argc > 3) ? std::stoi(argv[3]) : 1;
    
    std::string imageFilenameOrURL;
     #ifdef FLEXIUSE
       imageFilenameOrURL  = "https://i.etsystatic.com/37370207/r/il/8322f3/5483389650/il_1588xN.5483389650_7hvp.jpg";
    #else
       imageFilenameOrURL = "./images/imagedress.jpg";
    #endif

    std::string oldDirectoryPath = "./images/"; // Change this to your desired directory
    std::mutex resultsMutex; // Mutex for protecting access to the results vector
    std::vector<std::future<void>> workers;

    int invocationsPerThread = totalInvocations / threads;
    int remainingInvocations = totalInvocations % threads;

   for (int t = 0; t < threads; ++t) {
//        std::cout << "Thread " << t << " started" << std::endl;
       int invocationsForThisThread = invocationsPerThread + (t < remainingInvocations ? 1 : 0);
        workers.push_back(std::async(std::launch::async,
                processImagesPerThread,
                std::ref(t),
                std::ref(invocationsForThisThread),
                std::ref(imagesPerInvocation),
                std::ref(imageFilenameOrURL),
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
     std::cout << "flexi-serverless " << threads << " " << totalInvocations << " " << imagesPerInvocation << " " << duration << "\n";
     #else
     std::cout << "local " << threads << " " << totalInvocations << " " << imagesPerInvocation << " " << duration << "\n";
     #endif

//   for (const auto &result : results) {
//       std::cout << "Result for id \"" << std::get<0>(result) << "\" is \"" << std::get<1>(result) <<  " " << std::get<2>(result) << std::endl;
//   }

   return 0;
}
