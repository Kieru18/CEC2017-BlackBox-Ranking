#ifndef APIKEYMANAGER_H
#define APIKEYMANAGER_H

#include <iostream>
#include <random>
#include <sstream>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/NetException.h>


class ApiKeyManager {
public:
    std::string hashApiKey(const std::string& apiKey);
    std::string generateRandomApiKey(int length);
    void sendApiKey(const std::string& recipentAddress, const std::string& apiKey);
};

#endif // APIKEYMANAGER_H
