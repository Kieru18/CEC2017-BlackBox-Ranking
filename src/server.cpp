#include "server.h"
#include "db.h"

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>

template<typename T>
T parseDataFromJson(const std::string& json_data, const std::string& child_key){
    T result;
    std::stringstream error_stream;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        result = root.get_child(child_key).get_value<T>();
        
    } catch (const std::exception& e) {
        error_stream << "Error parsing JSON: " << e.what();
        throw std::runtime_error(error_stream.str());
    }

    return result;
}

const std::string generateHttpResponse(const std::string& text_response){
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: " + std::to_string(text_response.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                text_response;
    return response;
}

void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path) {
    std::stringstream error_stream;
    try {

        boost::asio::streambuf request;
        boost::system::error_code error;
        boost::asio::read_until(*socket, request, "\r\n\r\n", error);

        if (error && error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }

        std::istream request_stream(&request);
        std::string request_line, method, api_path;
        std::getline(request_stream, request_line);
        std::istringstream request_line_stream(request_line);
        request_line_stream >> method >> api_path;

        std::string line;

        while (std::getline(request_stream, line) && line != "\r") {}

        std::string body(std::istreambuf_iterator<char>(request_stream), {});

        std::string response;

        if (method == "POST" && api_path == "/registrate") {
            std::string mail = parseDataFromJson<std::string>(body, "mail");
            std::cout << "Użytkownik o adresie mailowym " << mail <<" wyraża chęć rejestracji\n";

            const bool is_user_registrated = isUserRegistratedInDatabase(mail, credentials_path);

            if (is_user_registrated)
            {
                response = generateHttpResponse("Istnieje użytkownik z takim adresem email (lub wystąpił jakiś błąd), nie możesz założyć konta\n");
            }
            else
            {
                response = generateHttpResponse("Nie istnieje użytkownik z takim adresem email, więc możesz założyć konto. Na podany adres e-mail dostaniesz klucz api (kiedyś)\n");
            }
        }
        else {
            response = generateHttpResponse("404 Not Found");
        }

        boost::asio::write(*socket, boost::asio::buffer(response), error);
        if (error) {
            throw boost::system::system_error(error);
        }

    } catch (std::exception& e) {
        error_stream << "Exception in thread: " << e.what();
        throw std::runtime_error(error_stream.str());

    }
}
