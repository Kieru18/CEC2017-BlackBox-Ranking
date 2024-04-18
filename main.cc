#include <boost/asio.hpp>
#include <iostream>
#include <sstream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>  // Include this header for std::accumulate
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <ctime> // ctime()



#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/FilePartSource.h>

#include <random>
#include <string>

using boost::asio::ip::tcp;

using namespace Poco::Net;
using namespace std;

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
        cerr << e.what() << ", message: " << e.message() << endl;
    }
    catch( const NetException & e )
    {
        cerr << e.what() << endl;
    }
}

std::unique_ptr<sql::ResultSet> getDatabaseResult(const std::string& request){
        sql::Driver *driver;
        sql::Connection *connection;
        sql::Statement *statement;
        try {

        // Create a MySQL driver instance
        driver = get_driver_instance();

        // Establish a connection to the MySQL database
        connection = driver->connect("tcp://127.0.0.1:3306", "zpr_user", "zpr_password");

        // Use the specific database
        connection->setSchema("zpr_example_database");

        // Execute SQL query
        statement = connection->createStatement();
        return std::unique_ptr<sql::ResultSet>(statement->executeQuery(request));
        }
        
        catch (sql::SQLException &e) {
            std::cerr << "SQL Error: " << e.what() << std::endl;
        }
        return nullptr;

}       




std::string get_user_data(const int& id){
    
    try {
        std::stringstream ssreq;
        ssreq << "SELECT * FROM zpr_example_table where id = " << id;
        std::unique_ptr<sql::ResultSet> database_request_result = getDatabaseResult(ssreq.str());

     if (database_request_result && database_request_result->next()) {
            // Pobieranie danych z wyniku zapytania
            int fetched_id = database_request_result->getInt("id"); // Można użyć getInt z nazwą kolumny
            std::string name = database_request_result->getString("name");

            // Tworzenie wynikowego stringa z danymi użytkownika
            std::stringstream ssres;
            ssres << "ID: " << fetched_id << " name: " << name;
            std::string user_data_result = ssres.str();

            return user_data_result;
        } else {
            // Jeśli zapytanie nie zwróciło żadnych rekordów
            return "ERROR - No user data found for ID: " + std::to_string(id);
        }
    } catch (sql::SQLException &e) {
        std::cerr << "SQL Error: " << e.what() << std::endl;
        return "ERROR - SQL Exception occurred";
    }
}

// Forward declaration for perform_calculation function
double perform_calculation(const std::string& function, const std::vector<double>& numbers);

// Function to parse numbers from a JSON string
std::vector<double> parse_numbers_from_json(const std::string& json_data) {
    std::vector<double> numbers;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        for (const auto& item : root.get_child("numbers")) {
            numbers.push_back(item.second.get_value<double>());
        }
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return numbers;
}

int parse_id_number_from_json(const std::string& json_data) {
    int id_number = -1;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        
        id_number = root.get_child("id").get_value<int>();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return id_number;
}

template<typename T>
T parseDataFromJson(const std::string& jsonData, const std::string& childKey){
    T result;
    try {
        boost::property_tree::ptree root;
        std::istringstream jsonStream(jsonData);
        boost::property_tree::read_json(jsonStream, root);
        result = root.get_child(childKey).get_value<T>();
        
    } catch (const std::exception& e) {
        std::cerr << "Error parsing JSON: " << e.what() << std::endl;
    }
    return result;

}


std::string generateHTTPResponse(const std::string& textResponse){
    std::string response =  "HTTP/1.1 200 OK\r\n"
                            "Content-Type: text/plain\r\n"
                            "Content-Length: " + std::to_string(textResponse.length()) + "\r\n"
                            "Connection: close\r\n\r\n" +
                            textResponse;
    return response;
}

void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        boost::asio::streambuf request;
        boost::system::error_code error;

        // Reading the request
        boost::asio::read_until(*socket, request, "\r\n\r\n", error);
        if (error && error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }

        // Extract the request as a string
        std::istream request_stream(&request);
        std::string request_line, method, path;
        std::getline(request_stream, request_line);
        std::istringstream request_line_stream(request_line);
        request_line_stream >> method >> path;

        // Skip headers
        std::string line;
        while (std::getline(request_stream, line) && line != "\r") {}

        // Read JSON data
        std::string body(std::istreambuf_iterator<char>(request_stream), {});

        // Prepare response
        std::string response;

        if (method == "POST" && path == "/registrate") {
            std::string mail = parseDataFromJson<std::string>(body, "mail");
            std::cout << "Użytkownik o adresie mailowym " << mail <<" wyraża chęć rejestracji\n";
            std::stringstream ssUsersTableRequest;
            ssUsersTableRequest <<"SELECT COUNT(*) AS users_count FROM zpr_users_table WHERE mail = " << "'" << mail <<"'";
            std::unique_ptr<sql::ResultSet> databaseRequestResult = getDatabaseResult(ssUsersTableRequest.str());
            if (databaseRequestResult && databaseRequestResult->next()) {
                int numOfUsersWithThatMail = databaseRequestResult->getInt("users_count");
                std::cout << "tyle jest obecnie użytkowników o takim mailu " << numOfUsersWithThatMail << "\n";
                if (numOfUsersWithThatMail != 0 )
                {
                    response = generateHTTPResponse("Istnieje użytkownik z takim adresem email (lub wystąpił jakiś błąd), nie możesz założyć konta\n");
                }
                else{
                    response = generateHTTPResponse("Nie istnieje użytkownik z takim adresem email, więc możesz założyć konto. Na podany adres e-mail dostaniesz klucz api\n");
                    std::string generatedApikey = generateRandomApiKey(20);
                    sendApiKey(mail, generatedApikey);
                    std::string hashedApiKey = hashApiKey(generatedApikey);
                    std::cout << "Oto zahashowany klucz api: " << hashedApiKey << "\n";

                    std::stringstream ssAddUserToTable;
                    ssAddUserToTable << "INSERT INTO zpr_users_table (mail, hashed_api_key, spend) VALUES " << "('"<<mail<<"', '"<<hashedApiKey<<"' ,"<<0<<");";
                    std::cout << ssAddUserToTable.str()<<"\n";
                    getDatabaseResult(ssAddUserToTable.str());

                    std::cout << "Dodano użytkownika " <<mail<<" do tablicy z bazy danych\n";
                }
            }
            else {
                std::cerr << "Wystąpił błąd z pobieraniem listy użytkowników\n";
            }
        }
        else if (method == "POST" && path == "/calculate") {
            // Parse JSON data
            std::vector<double> numbers = parse_numbers_from_json(body);
            std::string function_name = "unknown";
            try {
                boost::property_tree::ptree root;
                std::istringstream json_stream(body);
                boost::property_tree::read_json(json_stream, root);
                function_name = root.get<std::string>("function");
            } catch (const std::exception& e) {
                std::cerr << "Error parsing JSON: " << e.what() << std::endl;
            }

            std::cout << "Function Name: " << function_name << std::endl;
            std::cout << "JSON Data: " << body << std::endl;

            // Perform calculation based on function name
            double result = perform_calculation(function_name, numbers);

            std::cout << "Result: " << result << std::endl;

            // Prepare response
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: " + std::to_string(std::to_string(result).length()) + "\r\n"
                       "Connection: close\r\n\r\n" +
                       std::to_string(result);
        } 
        else if(method == "POST" && path == "/get_username") {
            int id_number = parse_id_number_from_json(body);
            std::cout << "Got 'get_username' request for id: " << id_number <<"\n";
            std::string id_question_result = get_user_data(id_number);
            response = "HTTP/1.1 200 OK\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: " + std::to_string(id_question_result.length()) + "\r\n"
                       "Connection: close\r\n\r\n" +
                       id_question_result;
        }
        else {
            response = "HTTP/1.1 404 Not Found\r\n"
                       "Content-Type: text/plain\r\n"
                       "Content-Length: 13\r\n"
                       "Connection: close\r\n\r\n"
                       "404 Not Found";
        }

        // Sending the response
        boost::asio::write(*socket, boost::asio::buffer(response), error);
        if (error) {
            throw boost::system::system_error(error);
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}




// Definition for perform_calculation function
double perform_calculation(const std::string& function, const std::vector<double>& numbers) {
    if (function == "sum") {
        return std::accumulate(numbers.begin(), numbers.end(), 0.0);
    } else if (function == "average") {
        if (!numbers.empty()) {
            return std::accumulate(numbers.begin(), numbers.end(), 0.0) / numbers.size();
        }
    }
    return 2000; // Arbitrary number for unknown functions or empty list

}









// Main function
int main() {
    //send_api_key("metyldaa@gmail.com", "miewam problemy z oddychaniem");
    try {
        boost::asio::io_context io_context;

        // Setting up the server to listen on TCP port 8080
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "Server is listening on port 8080...\n";

        // Infinite loop to serve requests
        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            // Handle the client in a separate thread
            std::thread(handle_client, socket).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}