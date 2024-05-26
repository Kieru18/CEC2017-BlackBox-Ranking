#include <iostream>
#include <memory>
#include "src/mail.h"
#include "src/db.h"
#include "src/server.h"
#include <boost/asio.hpp>
#include <thread>


int main() {
    try {
        const std::string credentials_path = "../bartosz_credentials.json";

        boost::asio::io_context io_context;

        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
        std::cout << "Server is listening on port 8080...\n";

        while (true) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
            acceptor.accept(*socket);
            std::thread(handleClient, socket, credentials_path).detach();

        }
    } catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}