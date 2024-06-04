// Jakub Kieruczenko, Bartosz Jaźwiec
// class handling api key creation and hashing

#include <iostream>
#include <iomanip>
#include <sstream>
#include <string>
#include <random>

#include <openssl/evp.h>

#include "ApiKeyManager.h"


// SHA-256 using new OpenSSL API
const std::string ApiKeyManager::hashGivenString(const std::string& text) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int lengthOfHash = 0;

    EVP_MD_CTX* context = EVP_MD_CTX_new();
    if (context == nullptr) {
        throw std::runtime_error("Failed to create context for hashing");
    }

    if (EVP_DigestInit_ex(context, EVP_sha256(), nullptr) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to initialize digest");
    }

    if (EVP_DigestUpdate(context, text.c_str(), text.size()) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to update digest");
    }

    if (EVP_DigestFinal_ex(context, hash, &lengthOfHash) != 1) {
        EVP_MD_CTX_free(context);
        throw std::runtime_error("Failed to finalize digest");
    }

    EVP_MD_CTX_free(context);

    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    for (unsigned int i = 0; i < lengthOfHash; ++i) {
        ss << std::setw(2) << static_cast<unsigned int>(hash[i]);
    }
    return ss.str();
}

const std::string ApiKeyManager::generateApiKey(int api_key_lenght){

    const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

    std::string api_key;

    for (int i = 0; i < api_key_lenght; ++i) {
        api_key += characters[distribution(generator)];
    }

    return api_key;
}
