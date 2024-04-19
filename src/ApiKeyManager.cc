#include <iostream>
#include <random>
#include <sstream>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/MailRecipient.h>
#include <Poco/Net/NetException.h>

using namespace Poco::Net;

class ApiKeyManager {
public:
    // Bardziej skomplikowana funkcja haszująca klucz API (tylko dla celów demonstracyjnych!)
    std::string hashApiKey(const std::string& apiKey) {
        std::string hashedKey;

        // Pętla po każdym znaku w kluczu API
        for (char c : apiKey) {
            // Operacje na znakach dla generowania złożonego "haszowania"
            char hashedChar = ((((c - 'a') * 3 + 5) % 26) +26)%26 + 'a';
            hashedKey += hashedChar;
        }

        return hashedKey;
    }

    std::string generateRandomApiKey(int length) {
        // Dostępne znaki (tylko małe i duże litery alfabetu angielskiego)
        const std::string characters = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";

        std::random_device rd;
        std::mt19937 generator(rd());
        std::uniform_int_distribution<int> distribution(0, characters.size() - 1);

        std::string apiKey;
        for (int i = 0; i < length; ++i) {
            apiKey += characters[distribution(generator)];
        }
        return apiKey;
    }

    void sendApiKey(const std::string& recipentAddress, const std::string& apiKey){

        std::string smtpServerAddress = "poczta.int.pl";
        std::string userLogin = "admin.zpr@int.pl";
        std::string senderAddress = userLogin;
        std::string userPassword = "zpr_password";
        std::stringstream statement;
        statement <<  "Witaj, oto twój klucz api: " << apiKey << "\nNie podawaj go nikomu i nie odpowiadaj na ten mail.\n\nZ poważaniem,\nGrzegorz Florianowicz";
        try
        {
            MailRecipient recipient1( MailRecipient::PRIMARY_RECIPIENT, recipentAddress );
        
            MailMessage message;
            message.setRecipients( { recipient1 } );
            message.setSubject( MailMessage::encodeWord( "Ściśle tajny klucz API od CEC2017-BlackBox-Ranking" ) );
            message.setContentType( "multipart/mixed; charset=utf-8;" );
            message.setSender( senderAddress );
            message.addContent( new StringPartSource( statement.str() ) );
        
            SecureSMTPClientSession session( smtpServerAddress );
            session.login();
            session.startTLS();
            session.login( SMTPClientSession::AUTH_PLAIN, userLogin, userPassword );
            session.sendMessage( message );
            session.close();
            std::cout << "Mail sent";
        }
        catch( const SMTPException & e )
        {
            std::cerr << e.what() << ", message: " << e.message() << std::endl;
        }
        catch( const NetException & e )
        {
            std::cerr << e.what() << std::endl;
        }
    }
};