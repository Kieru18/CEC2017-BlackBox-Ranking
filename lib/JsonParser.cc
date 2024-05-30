#include <iostream>
#include <sstream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "JsonParser.hpp"


int JsonParser::parseIdNumberFromJson(const std::string& json_data) {
    int id_number = -1;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        id_number = root.get_child("id").get_value<int>();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return id_number;
}

// Function to parse numbers from a JSON string
std::vector<double> JsonParser::parseNumbersFromJson(const std::string& json_data) {
    std::vector<double> numbers;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        for (const auto& item : root.get_child("numbers")) {
            numbers.push_back(item.second.get_value<double>());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return numbers;
}
