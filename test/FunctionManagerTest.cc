#include <gtest/gtest.h>
#include "../lib/FunctionManager.h"


TEST(FunctionManagerTest, TestSingleSpecimen) {
    std::vector<double> specimen = {1.0, 2.0};
    double expected = 0.0;
    FunctionManager functionManager;
    double result = functionManager.getFunctionResults(2, specimen);
    ASSERT_EQ(result, expected);
}

TEST(FunctionManagerTest, TestPopulationResults) {
    std::vector<std::vector<double>> population = {
        {1.0, 2.0},
        {4.0, 5.0}
    };
    std::vector<double> expected = {0.0, 0.0}; 
    FunctionManager functionManager;
    std::vector<double> results = functionManager.getFunctionResults(2, population);
    ASSERT_EQ(results, expected);
}
