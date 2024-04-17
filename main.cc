#include <iostream>
#include <boost/asio.hpp>

using namespace boost::asio;
using ip::tcp;

void handle_request(tcp::socket& socket) {
    boost::system::error_code error;
    char buffer[1024];

    // Read the request from the client
    size_t bytes_transferred = socket.read_some(buffer, error);

    if (!error) {
        // Send a response back to the client
        std::string response = "HTTP/1.1 200 OK\r\nContent-Length: 13\r\n\r\nHello, World!";
        socket.write_some(buffer(response.c_str(), response.size()), error);
    }
}

int main() {
    io_service io_service;
    tcp::acceptor acceptor(io_service, tcp::endpoint(tcp::v4(), 8080));

    while (true) {
        tcp::socket socket(io_service);
        acceptor.accept(socket);

        // Handle the request in a separate thread
        std::thread(handle_request, std::move(socket)).detach();
    }

    return 0;
}