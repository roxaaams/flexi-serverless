#include <iostream>
#include <thread>

int main() {
    unsigned int maxThreads = std::thread::hardware_concurrency();
    if (maxThreads == 0) {
        std::cout << "Unable to determine the number of threads." << std::endl;
    } else {
        std::cout << "Maximum number of threads available: " << maxThreads << std::endl;
    }
    return 0;
}
