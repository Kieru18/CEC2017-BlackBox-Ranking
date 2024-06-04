// Jakub Kieruczenko, Bartosz Jaźwiec
// wrapper class for cec2017 functions

#pragma once
#include <string>
#include <vector>
#include <numeric>

class FunctionManager {
public:
    FunctionManager() = default;
    ~FunctionManager() = default;
    double getFunctionResults(const int function_number, const std::vector<double>& specimen);
    std::vector<double> getFunctionResults(const int function_number, const std::vector<std::vector<double>>& population);
};
