#include <string>
#include <vector>
#include <numeric>
#include "FunctionManager.h"

// Definition for perform_calculation function
double FunctionManager::perform_calculation(const std::string& function, const std::vector<double>& numbers) {
    if (function == "sum") {
        return std::accumulate(numbers.begin(), numbers.end(), 0.0);
    } else if (function == "average") {
        if (!numbers.empty()) {
            return std::accumulate(numbers.begin(), numbers.end(), 0.0) / numbers.size();
        }
    }

    return 2137; // Arbitrary number for unknown functions or empty list
}
