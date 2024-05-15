#ifndef MAIL_H
#define MAIL_H

#include <string>
#include <tuple>

std::tuple<std::string, std::string, std::string> getMailCredentials(const std::string& path);
void sendMail(const std::string& recipent_address, const std::string& subject, const std::string& content, const std::string& path);

#endif
