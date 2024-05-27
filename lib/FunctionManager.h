#pragma once
#include <string>
#include <vector>
#include <numeric>

class FunctionManager {
public:
    FunctionManager() = default;
    ~FunctionManager() = default;
    double perform_calculation(const std::string& function, const std::vector<double>& numbers);
};
