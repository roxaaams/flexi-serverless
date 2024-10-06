#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <algorithm>

// Function to generate a random string of given length from the alphabet
std::string generate_random_string(size_t length) {
    const std::string alphabet = "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<> distribution(0, alphabet.size() - 1);

    std::string random_string;
    for (size_t i = 0; i < length; ++i) {
        random_string += alphabet[distribution(generator)];
    }
    return random_string;
}

// Function to generate pairs of strings, some similar or equal, others different
std::vector<std::pair<std::string, std::string>> generate_string_pairs(size_t num_pairs) {
    std::vector<std::pair<std::string, std::string>> pairs;
    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_real_distribution<> distribution(0.0, 1.0);
    std::uniform_int_distribution<> length_distribution(3, 100);

    for (size_t i = 0; i < num_pairs; ++i) {
        size_t string_length = length_distribution(generator);
        std::string str1 = generate_random_string(string_length);
        std::string str2;

        if (distribution(generator) < 0.5) {
            if (distribution(generator) < 0.5) {
                str2 = str1;  // Equal
            } else {
                // Similar: change a few characters
                str2 = str1;
                std::uniform_int_distribution<> char_distribution(0, string_length - 1);
                for (size_t j = 0; j < std::uniform_int_distribution<>(1, 3)(generator); ++j) {
                    str2[char_distribution(generator)] = generate_random_string(1)[0];
                }
            }
        } else {
            str2 = generate_random_string(string_length);  // Completely different
        }
        pairs.emplace_back(str1, str2);
    }
    return pairs;
}

// Function to write pairs of strings to a file, each pair on a new line separated by a space
void write_pairs_to_file(const std::vector<std::pair<std::string, std::string>>& pairs, const std::string& filename) {
    std::ofstream file(filename);
    if (file.is_open()) {
        for (const auto& pair : pairs) {
            file << pair.first << " " << pair.second << "\n";
        }
        file.close();
    } else {
        std::cerr << "Unable to open file: " << filename << std::endl;
    }
}

int main() {
    size_t num_pairs = 100000;
    std::vector<std::pair<std::string, std::string>> pairs = generate_string_pairs(num_pairs);

    // Write the pairs to a file
    write_pairs_to_file(pairs, "string_pairs.txt");

    return 0;
}
