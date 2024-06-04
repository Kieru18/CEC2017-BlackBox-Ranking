// Jakub Kieruczenko, Bartosz Jaźwiec
// class managing email operations

#pragma once

#include <string>
#include <tuple>


class MailManager {
public:
    MailManager() = default;
    ~MailManager() = default;
    bool isEmailValid(const std::string& email);
    std::tuple<std::string, std::string, std::string> getMailCredentials(const std::string& path);
    void sendMail(const std::string& recipent_address, const std::string& subject, const std::string& content, const std::string& path);
    void sendRejectionMail(const std::string& recipent_address, const std::string& path);
    void sendAcceptanceMail(const std::string& recipent_address, const std::string& api_key, const std::string& path);
};
