#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <filesystem>
#include <thread>
#include <chrono>

#include <nlohmann/json.hpp>

#include "base64.h"
#include "flexi.h"

using json = nlohmann::json;

std::string functionName = "imagefiltering-dev";

void processImage(const std::string& inputFileName, const std::string& outputFileName, int index) {
    std::ofstream imageInfo;

    imageInfo.open(outputFileName);
    if (!imageInfo) {
        std::cerr << "Error: Failed to open the output file: " << outputFileName << std::endl;
        return;
    }

    int whitePixels = 0;
    int blackPixels = 0;

    #ifdef FLEXIUSE
        std::string payload = "{\"image\": \"" + inputFileName +"\"}";
        std::string response = invoke(functionName, payload, index);
        std::cout << response << std::endl;
        // Parse the JSON string
        json jsonData = json::parse(response);
        // Extract the body and parse it as JSON
        std::string bodyString = jsonData["body"];
        json bodyData = json::parse(bodyString);
        whitePixels = bodyData["whitePixels"].get<int>();
        blackPixels = bodyData["blackPixels"].get<int>();
        std::cout << "White Pixels: " << whitePixels << std::endl;
        std::cout << "Black Pixels: " << blackPixels << std::endl;

        imageInfo << whitePixels << " " << blackPixels << std::endl;
    #else
        std::ifstream image;
        image.open(inputFileName);

        // Copy over header information
        std::string type = "", width = "", height = "", RGB = "";
        image >> type >> width >> height >> RGB;

        std::string red = "", green = "", blue = "";
        int r = 0, g = 0, b = 0;
        int white = 0, black = 0;

        while (!image.eof()) {
            image >> red >> green >> blue;
            std::stringstream redStream(red);
            std::stringstream greenStream(green);
            std::stringstream blueStream(blue);

            redStream >> r;
            greenStream >> g;
            blueStream >> b;

            // Assuming that if R, G, and B components are equal, it's a grayscale pixel
            if (r === g && g === b) {
                if (r === 0) {
                    blackPixels++;
                } else if (r === 255) {
                    whitePixels++;
                }
            }
         }
         imageInfo << whitePixels << " " << blackPixels << std::endl;
         image.close();
    #endif
    imageInfo.close();
}

int main() {
    std::string oldDirectoryPath = "./images/"; // Change this to your desired directory
    std::string newDirectoryPath = "./info/"; // Change this to your desired directory

    std::vector<std::string> inputImages;
    std::vector<std::string> outputImages;

    std::string imageNamesFile = "";

    #ifdef FLEXIUSE
      imageNamesFile = "image_urls.txt";
    #else
       imageNamesFile = "image_names.txt";
    #endif

   std::ifstream imageNamesStream(imageNamesFile);
   if (!imageNamesStream) {
       std::cerr << "Error: Failed to open the image names file: " << imageNamesFile << std::endl;
       return 1;
   }

   std::string imageName;
   int i =0;
   while (imageNamesStream >> imageName) {
        #ifdef FLEXIUSE
           inputImages.push_back(imageName);
           outputImages.push_back(newDirectoryPath + "image" + std::to_string(i) + ".txt");
        #else
           inputImages.push_back(oldDirectoryPath + imageName);
           outputImages.push_back(newDirectoryPath + imageName + ".txt");
        #endif
        i++;
   }

   imageNamesStream.close();


    std::vector<std::thread> threads;

    for (size_t i = 0; i < inputImages.size(); ++i) {
        threads.emplace_back(processImage, inputImages[i], outputImages[i], i);
    }

    // Wait for all threads to finish
    auto startTime = std::chrono::high_resolution_clock::now();
    for (auto& thread : threads) {
        thread.join();
    }
     auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime).count();
    std::cout << "Elapsed time: " << duration << " milliseconds\n";

    return 0;
}
