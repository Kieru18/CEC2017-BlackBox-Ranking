#include <iostream>
#include <boost/asio.hpp>
#include <string>

using boost::asio::ip::tcp;

int main() {
    try {
        boost::asio::io_context io_context;

        // Resolve endpoint
        tcp::resolver resolver(io_context);
        tcp::resolver::results_type endpoints = resolver.resolve("localhost", "8080");

        // Establish connection
        tcp::socket socket(io_context);
        boost::asio::connect(socket, endpoints);

        // Prepare request JSON
        std::string request = R"({"function": "sum", "numbers": [1, 7, 3]})";

        // Send request
        boost::asio::write(socket, boost::asio::buffer("POST /calculate HTTP/1.1\r\n"));
        boost::asio::write(socket, boost::asio::buffer("Host: localhost\r\n"));
        boost::asio::write(socket, boost::asio::buffer("Content-Type: application/json\r\n"));
        boost::asio::write(socket, boost::asio::buffer("Content-Length: " + std::to_string(request.size()) + "\r\n"));
        boost::asio::write(socket, boost::asio::buffer("\r\n"));
        boost::asio::write(socket, boost::asio::buffer(request));

        // Read response
        boost::asio::streambuf response_buf;
        boost::asio::read_until(socket, response_buf, "\r\n");
        std::istream response_stream(&response_buf);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);

        if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
            std::cerr << "Invalid response\n";
            return 1;
        }

        if (status_code != 200) {
            std::cerr << "Response returned with status code " << status_code << "\n";
            return 1;
        }

        // Read and print response body
        boost::asio::read_until(socket, response_buf, "\r\n\r\n");
        std::string header;
        while (std::getline(response_stream, header) && header != "\r") {}
        std::ostringstream response_data;
        if (response_buf.size() > 0) {
            response_data << &response_buf;
        }
        std::cout << "Response from server: " << response_data.str() << "\n";

    } catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
