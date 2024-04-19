#ifndef FUNCTIONMANAGER_H
#define FUNCTIONMANAGER_H

#include <string>
#include <vector>
#include <numeric>

class FunctionManager {
public:
    double perform_calculation(const std::string& function, const std::vector<double>& numbers);
};

#endif // FUNCTIONMANAGER_H