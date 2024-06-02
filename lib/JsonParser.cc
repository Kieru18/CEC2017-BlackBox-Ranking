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

std::vector<std::vector<double>> JsonParser::parsePopulationFromJson(const std::string& json_data) {
    std::vector<std::vector<double>> result;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        for (const auto& population : root.get_child("population")) {
            std::vector<double> individual;
            for (const auto& item : population.second) {
                individual.push_back(item.second.get_value<double>());
            }
            result.push_back(individual);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return result;
}


std::vector<double> JsonParser::parseSpecimenFromJson(const std::string& json_data) {
    std::vector<double> specimen;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        for (const auto& item : root.get_child("specimen")) {
            specimen.push_back(item.second.get_value<double>());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return specimen;
}