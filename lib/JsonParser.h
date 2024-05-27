#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class JsonParser {
public:
    JsonParser() = default;
    ~JsonParser() = default;
    int parseIdNumberFromJson(const std::string& json_data);
    std::vector<double> parseNumbersFromJson(const std::string& json_data);
    template<typename T>
    T parseDataFromJson(const std::string& jsonData, const std::string& childKey);
};
