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
    
    template<typename T>
    T parseDataFromJson(const std::string& jsonData, const std::string& childKey);

    std::vector<std::vector<double>> parsePopulationFromJson(const std::string& json_data);
    std::vector<double> parseSpecimenFromJson(const std::string& json_data);
};

template<typename T>
    T JsonParser::parseDataFromJson(const std::string& jsonData, const std::string& childKey) {
    T result;
    try {
        boost::property_tree::ptree root;
        std::istringstream jsonStream(jsonData);
        boost::property_tree::read_json(jsonStream, root);
        result = root.get_child(childKey).get_value<T>();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }

    return result;
}
