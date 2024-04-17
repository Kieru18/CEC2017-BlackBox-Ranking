#include <iostream>
#include <boost/asio.hpp>
#include <string>

using boost::asio::ip::tcp;

std::string generate_request() {
    std::string json_data = "{\"function\": \"sum\", \"numbers\": [1, 7, 3]}";
    return "POST /calculate HTTP/1.1\r\n"
           "Host: localhost:8080\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: " + std::to_string(json_data.length()) + "\r\n"
           "Connection: close\r\n\r\n"
           + json_data;
}

int main() {
    try {
        boost::asio::io_context io_context;
        tcp::socket socket(io_context);
        tcp::resolver resolver(io_context);

        boost::asio::connect(socket, resolver.resolve("localhost", "8080"));

        // Prepare request
        std::string request = generate_request();

        // Send request
        boost::asio::write(socket, boost::asio::buffer(request));

        // Read response
        boost::asio::streambuf response;
        boost::asio::read_until(socket, response, "\r\n");

        // Process and print response
        std::istream response_stream(&response);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;

        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cout << "Invalid response\n";
            return 1;
        }

        if (status_code != 200) {
            std::cout << "Response returned with status code " << status_code << "\n";
            return 1;
        }

        // Read the rest of the response
        boost::asio::read_until(socket, response, "\r\n\r\n");

        // Output the response
        std::cout << &response << std::endl;

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
