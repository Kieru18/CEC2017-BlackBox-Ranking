#include <iostream>
#include <sstream>
#include <vector>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <nlohmann/json.hpp>

#include "JsonParser.hpp"


int JsonParser::parseIdNumberFromJson(const std::string& json_data) {
    int id_number = -1;
    try {
        nlohmann::json root = nlohmann::json::parse(json_data);
        id_number = root["id"].get<int>();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return id_number;
}

std::vector<std::vector<double>> JsonParser::parsePopulationFromJson(const std::string& json_data) {

    std::vector<std::vector<double>> result;
    try {
        nlohmann::json root = nlohmann::json::parse(json_data);
        for (const auto& population : root["population"]) {
            std::vector<double> individual;
            for (const auto& item : population) {
                individual.push_back(item);
            }
            result.push_back(individual);
        }
    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return result;
}


std::vector<double> JsonParser::parseSpecimenFromJson(const std::string& json_data) {
    std::vector<double> specimen;
    try {
        std::cout<<json_data<<"\n";
        nlohmann::json root = nlohmann::json::parse(json_data);
        std::cout<<root["specimen"].size()<<"\n";
        for (size_t i = 0; i < root["specimen"].size(); ++i) {
            specimen.push_back(root["specimen"][i]);
         }

    } catch (const nlohmann::json::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return specimen;

}