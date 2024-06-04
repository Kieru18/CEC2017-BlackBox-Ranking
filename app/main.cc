#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <ctime>
#include <random>
#include <string>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/StreamSocket.h>
#include <Poco/Net/SocketAddress.h>

#include "../lib/HttpRequestHandler.h"


int main() {
    try {
        const std::string CREDENTIALS_PATH = "credentials.json"; // this should be in config file?

        boost::asio::io_context io_context;
        
        std::shared_ptr<HttpRequestHandler> handler = std::make_shared<HttpRequestHandler>();
        

        boost::asio::ip::tcp::acceptor acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), 8080));
        std::cout << "Server is listening on port 8080...\n";

        while (true) {
            auto socket = std::make_shared<boost::asio::ip::tcp::socket>(io_context);
            acceptor.accept(*socket);

            std::thread([&handler, socket, CREDENTIALS_PATH]() {
                handler->handleClient(socket, CREDENTIALS_PATH);
            }).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}