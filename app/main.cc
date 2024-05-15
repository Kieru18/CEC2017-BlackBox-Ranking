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

#include "../lib/headers/HttpRequestHandler.h"

using boost::asio::ip::tcp;

using namespace Poco::Net;

int main() {
    try {
        boost::asio::io_context io_context;
        
        std::shared_ptr<HttpRequestHandler> handler = std::make_shared<HttpRequestHandler>();
        

        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "Server is listening on port 8080...\n";

        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            std::thread([&handler, socket]() {
                handler->handleClient(socket);
            }).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}