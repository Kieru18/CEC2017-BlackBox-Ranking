#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

using boost::asio::ip::tcp;

// Function to handle the client request
void handle_client(std::shared_ptr<tcp::socket> socket) {
    try {
        boost::asio::streambuf request;
        boost::system::error_code error;

        // Reading the request
        while (boost::asio::read_until(*socket, request, "\r\n\r\n", error)) {
            if (error && error != boost::asio::error::eof) {
                throw boost::system::system_error(error);
            } else if (error == boost::asio::error::eof) {
                // Client closed the connection
                std::cerr << "Client closed the connection.\n";
                return;  // Clean exit on client connection close
            }

            // Prepare a response
            std::string response =
                "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/plain\r\n"
                "Content-Length: 12\r\n"
                "Connection: close\r\n\r\n"
                "Hello World!";

            // Sending a response
            boost::asio::write(*socket, boost::asio::buffer(response), error);
            if (error) {
                throw boost::system::system_error(error);
            }

            // Since the response has been sent and connection is to be closed,
            // break the loop to prevent further reads.
            break;
        }
    } catch (std::exception& e) {
        std::cerr << "Exception in thread: " << e.what() << "\n";
    }
}

int main() {
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
        std::cerr << "Exception: " << e.what() << "\n";
    }

    return 0;
}
