#include <gtest/gtest.h>
#include "../lib/ApiKeyManager.h"


TEST(ApiKeyManagerTest, GenerateApiKeyWithZeroLength) {
    ApiKeyManager manager;
    int length = 0;
    std::string apiKey = manager.generateApiKey(length);
    EXPECT_EQ(length, apiKey.length());
}

TEST(ApiKeyManagerTest, GenerateApiKeyWithNegativeLength) {
    ApiKeyManager manager;
    int length = -5;
    std::string apiKey = manager.generateApiKey(length);
    EXPECT_EQ(0, apiKey.length());
}


TEST(ApiKeyManagerTest, HashEmptyString) {
    ApiKeyManager manager;
    std::string input = "";
    std::string expectedOutput = "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855";
    std::string actualOutput = manager.hashGivenString(input);
    EXPECT_EQ(expectedOutput, actualOutput);
}


TEST(ApiKeyManagerTest, GenerateApiKey) {
    ApiKeyManager manager;
    int length = 16;
    std::string apiKey = manager.generateApiKey(length);
    EXPECT_EQ(length, apiKey.length());
}
