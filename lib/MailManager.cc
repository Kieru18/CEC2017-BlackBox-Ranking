#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <regex>
#include <string>
#include <tuple>

#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/FilePartSource.h>
#include <nlohmann/json.hpp>

#include "MailManager.h"


bool MailManager::isEmailValid(const std::string& email) {
    const std::regex pattern(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    return std::regex_match(email, pattern);
}

std::tuple<std::string, std::string, std::string> MailManager::getMailCredentials(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
    }   

    nlohmann::json data;
    file >> data;
    file.close();

    std::string server;
    std::string address;
    std::string password;

    if (data.find("mail") != data.end()) {
        auto mail_credentials = data["mail"];

        if (mail_credentials.find("server") != mail_credentials.end()) {
            server = mail_credentials["server"];
        }
        else{
            throw std::runtime_error("Mail server not found in credentials");
        }

        if (mail_credentials.find("address") != mail_credentials.end()) {
            address = mail_credentials["address"];
        }
        else{
            throw std::runtime_error("Mail address not found in credentials");
        }

        if (mail_credentials.find("password") != mail_credentials.end()) {
            password = mail_credentials["password"];
        }
        else{
            throw std::runtime_error("Mail password not found in credentials");
        }
    } 
    else 
    {
        throw std::runtime_error("Mail credentials not found in JSON");
    }
    return std::make_tuple(server, address, password);
}

void MailManager::sendMail(const std::string& recipent_address, const std::string& subject, const std::string& content, const std::string& path)
{
    std::stringstream error_stream;

    try {
        const std::tuple<std::string, std::string, std::string> credentials = getMailCredentials(path);
        const std::string server_address = std::get<0>(credentials);
        const std::string sender_address = std::get<1>(credentials);
        const std::string sender_password = std::get<2>(credentials);

        Poco::Net::MailRecipient recipient( Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipent_address );

        Poco::Net::MailMessage message;

        message.setRecipients({recipient});
        message.setSubject(Poco::Net::MailMessage::encodeWord(subject, "UTF-8"));
        message.setContentType("text/plain; charset=UTF-8");
        message.setSender(sender_address);
        message.addContent(new Poco::Net::StringPartSource(content, "text/plain; charset=UTF-8"));

        Poco::Net::SecureSMTPClientSession session( server_address );
        session.login();
        session.startTLS();
        session.login(Poco::Net::SMTPClientSession::AUTH_PLAIN, sender_address, sender_password);
        session.sendMessage(message);
        session.close();
        std::cout << "Wysłano maila\n";
    }
    catch( const Poco::Net::SMTPException & e ){

        error_stream << "Poco SMTPException: " << e.what() << ", message: " << e.message();
    }
    catch( const Poco::Net::NetException & e ){
        error_stream<< "Poco NetException error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        error_stream << "Sending mail error: " << e.what() << std::endl;
    }
    throw std::runtime_error(error_stream.str());

}

void MailManager::sendRejectionMail(const std::string& recipent_address, const std::string& path)
{
    const std::string content = R"(
        Szanowny Użytkowniku,
        Z niezmierną przykrością musimy zawiadomić że nie zaakceptowano Państwa udziału w naszym eksluzywnym konkursie.
        Życzymy powodzenia w aplikowaniu na następne edycje naszych prestiżowych zawodów.

        Z poważaniem,
        Administracja
    )";
    sendMail(recipent_address, "Odmowa zarejestrowania w konkursie", content, path);
}

void MailManager::sendAcceptanceMail(const std::string& recipent_address, const std::string& api_key, const std::string& path)
{
    const std::string content = 
        R"(
        Szanowny Użytkowniku,
        Zaakceptowano Państwa udział w naszym eksluzywnym konkursie.
        Życzymy powodzenia, oto klucz API, złożony z )" + 
        std::to_string(api_key.length()) + 
        R"( znaków: )" + 
        api_key + 
        R"(
        Z poważaniem,
        Administracja
        )";
    sendMail(recipent_address, "Akceptacja rejestracji w konkursie", content, path);
}
