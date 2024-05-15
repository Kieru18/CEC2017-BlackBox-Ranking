#include "mail.h"
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/FilePartSource.h>

#include "libs/json.hpp"

#include <string>
#include <tuple>

#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>


std::tuple<std::string, std::string, std::string> getMailCredentials(const std::string& path)
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
        std::cerr << "Mail server not found in credentials" << std::endl;
        }

        if (mail_credentials.find("address") != mail_credentials.end()) {
            address = mail_credentials["address"];
        }
        else{
        std::cerr << "Mail address not found in credentials" << std::endl;
        }

        if (mail_credentials.find("password") != mail_credentials.end()) {
            password = mail_credentials["password"];
        }
        else{
        std::cerr << "Mail password not found in credentials" << std::endl;
        }
    } 
    else 
    {
        std::cerr << "Mail credentials not found in JSON" << std::endl;
    }
    return std::make_tuple(server, address, password);
}



void sendMail(const std::string& recipent_address, const std::string& subject, const std::string& content, const std::string& path)
{
    try {
        std::tuple<std::string, std::string, std::string> credentials = getMailCredentials(path);
        std::string server_address = std::get<0>(credentials);
        std::string sender_address = std::get<1>(credentials);
        std::string sender_password = std::get<2>(credentials);

        Poco::Net::MailRecipient recipient( Poco::Net::MailRecipient::PRIMARY_RECIPIENT, recipent_address );

        Poco::Net::MailMessage message;

        message.setRecipients({recipient});
        message.setSubject(Poco::Net::MailMessage::encodeWord(subject));
        message.setContentType("multipart/mixed; charset=utf-8;");
        message.setSender(sender_address);
        message.addContent(new Poco::Net::StringPartSource(content));

        Poco::Net::SecureSMTPClientSession session( server_address );
        session.login();
        session.startTLS();
        session.login(Poco::Net::SMTPClientSession::AUTH_PLAIN, sender_address, sender_password);
        session.sendMessage(message);
        session.close();
        std::cout << "Mail sent\n";
    }
    catch( const Poco::Net::SMTPException & e ){
        std::cerr << "Poco SMTPException: " << e.what() << ", message: " << e.message() << std::endl;
    }
    catch( const Poco::Net::NetException & e ){
        std::cerr << "Poco NetException error: " << e.what() << std::endl;
    }
    catch (const std::exception& e) {
        std::cerr << "Sending mail error: " << e.what() << std::endl;
    }
    
}

