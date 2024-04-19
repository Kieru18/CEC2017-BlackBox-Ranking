#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "../headers/DatabaseManager.h"
#include "../headers/JsonParser.h"
#include "../headers/ApiKeyManager.h"
#include "../headers/FunctionManager.h"

using namespace Poco::Net;

class HttpRequestHandler {
private:
    DatabaseManager dbManager;
    JsonParser jsonParser;
    ApiKeyManager apiKeyManager;
    FunctionManager functionManager;

    std::string generateHTTPResponse(const std::string& textResponse) {
        std::string response =  "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: " + std::to_string(textResponse.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                textResponse;
        return response;
    }
public:
    HttpRequestHandler() : dbManager(), jsonParser(), apiKeyManager(), functionManager() {}

    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket) {
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

            if (method == "POST" && path == "/register") {
                std::string mail = jsonParser.parseDataFromJson<std::string>(body, "mail");
                std::cout << "Użytkownik o adresie mailowym " << mail <<" wyraża chęć rejestracji\n";
                std::stringstream ssUsersTableRequest;
                ssUsersTableRequest <<"SELECT COUNT(*) AS users_count FROM zpr_users_table WHERE mail = " << "'" << mail <<"'";
                std::unique_ptr<sql::ResultSet> databaseRequestResult = dbManager.getDatabaseResult(ssUsersTableRequest.str());
                if (databaseRequestResult && databaseRequestResult->next()) {
                    int numOfUsersWithThatMail = databaseRequestResult->getInt("users_count");
                    std::cout << "tyle jest obecnie użytkowników o takim mailu " << numOfUsersWithThatMail << "\n";
                    if (numOfUsersWithThatMail != 0 )
                    {
                        response = generateHTTPResponse("Istnieje użytkownik z takim adresem email (lub wystąpił jakiś błąd), nie możesz założyć konta\n");
                    }
                    else {
                        response = generateHTTPResponse("Nie istnieje użytkownik z takim adresem email, więc możesz założyć konto. Na podany adres e-mail dostaniesz klucz api\n");
                        std::string generatedApikey = apiKeyManager.generateRandomApiKey(20);
                        apiKeyManager.sendApiKey(mail, generatedApikey);
                        std::string hashedApiKey = apiKeyManager.hashApiKey(generatedApikey);
                        std::cout << "Oto zahashowany klucz api: " << hashedApiKey << "\n";

                        std::stringstream ssAddUserToTable;
                        ssAddUserToTable << "INSERT INTO zpr_users_table (mail, hashed_api_key, spend) VALUES " << "('"<<mail<<"', '"<<hashedApiKey<<"' ,"<<0<<");";
                        std::cout << ssAddUserToTable.str()<<"\n";
                        dbManager.getDatabaseResult(ssAddUserToTable.str());

                        std::cout << "Dodano użytkownika " <<mail<<" do tablicy z bazy danych\n";
                    }
                }
                else {
                    std::cerr << "Wystąpił błąd z pobieraniem listy użytkowników\n";
                }
            }
            else if (method == "POST" && path == "/calculate") {
                // Parse JSON data
                std::vector<double> numbers = jsonParser.parse_numbers_from_json(body);
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
                double result = functionManager.perform_calculation(function_name, numbers);

                std::cout << "Result: " << result << std::endl;

                // Prepare response
                response = "HTTP/1.1 200 OK\r\n"
                        "Content-Type: text/plain\r\n"
                        "Content-Length: " + std::to_string(std::to_string(result).length()) + "\r\n"
                        "Connection: close\r\n\r\n" +
                        std::to_string(result);
            } 
            else if(method == "POST" && path == "/get_username") {
                int id_number = jsonParser.parse_id_number_from_json(body);
                std::cout << "Got 'get_username' request for id: " << id_number <<"\n";
                std::string id_question_result = dbManager.get_user_data(id_number);
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

};