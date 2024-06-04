#include <string>
#include <vector>
#include <numeric>

#include "../cec_codes/cec17_test_func.h"

#include "FunctionManager.h"

double FunctionManager::getFunctionResults(const int function_number, const std::vector<double>& specimen) {
    double* x = new double[specimen.size()];
    std::copy(specimen.begin(), specimen.end(), x);
    double* f = new double[1];
    cec17_test_func(x, f, specimen.size(), 1, function_number);
    delete[] x;
    double result = f[0];
    delete[] f;
    return result;
}

std::vector<double> FunctionManager::getFunctionResults(const int function_number, const std::vector<std::vector<double>>& population) {
    std::vector<double> results;
    for (const auto& specimen : population) {
        results.push_back(getFunctionResults(function_number, specimen));
    }
    return results;
}
