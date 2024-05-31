#pragma once

#include <string>

class ApiKeyManager {
public:
    ApiKeyManager() = default;
    ~ApiKeyManager() = default;
    const std::string hashGivenString(const std::string& text);
    const std::string generateApiKey(int api_key_lenght);
};