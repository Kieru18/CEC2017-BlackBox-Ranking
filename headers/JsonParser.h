#pragma once
#include <iostream>
#include <sstream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

class JsonParser {
public:
    int parse_id_number_from_json(const std::string& json_data);
    std::vector<double> parse_numbers_from_json(const std::string& json_data);
    template<typename T>
    T parseDataFromJson(const std::string& jsonData, const std::string& childKey);
};
