#include <gtest/gtest.h>
#include "../lib/JsonParser.hpp"


TEST(JsonParserTest, TestParseIdNumberFromJson) {
    std::string json_data = R"({"id": 123})";
    JsonParser jsonParser;
    int id_number = jsonParser.parseIdNumberFromJson(json_data);
    ASSERT_EQ(id_number, 123);
}

TEST(JsonParserTest, TestParsePopulationFromJson) {
    std::string json_data = R"({"population": [[1.0, 2.0, 3.0], [4.0, 5.0, 6.0]]})";
    JsonParser jsonParser;
    std::vector<std::vector<double>> population = jsonParser.parsePopulationFromJson(json_data);
    ASSERT_EQ(population.size(), 2);
    ASSERT_EQ(population[0].size(), 3);
    ASSERT_EQ(population[1].size(), 3);
    ASSERT_EQ(population[0][0], 1.0);
    ASSERT_EQ(population[0][1], 2.0);
    ASSERT_EQ(population[0][2], 3.0);
    ASSERT_EQ(population[1][0], 4.0);
    ASSERT_EQ(population[1][1], 5.0);
    ASSERT_EQ(population[1][2], 6.0);
}

TEST(JsonParserTest, TestParseSpecimenFromJson) {
    std::string json_data = R"({"specimen": [1.0, 2.0, 3.0]})";
    JsonParser jsonParser;
    std::vector<double> specimen = jsonParser.parseSpecimenFromJson(json_data);
    ASSERT_EQ(specimen.size(), 3);
    ASSERT_EQ(specimen[0], 1.0);
    ASSERT_EQ(specimen[1], 2.0);
    ASSERT_EQ(specimen[2], 3.0);
}
