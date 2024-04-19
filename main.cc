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

#include "./headers/HttpRequestHandler.h"

using boost::asio::ip::tcp;

using namespace Poco::Net;


// Main function
int main() {
    try {
        boost::asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::shared_ptr<HttpRequestHandler> httpHandler = std::make_shared<HttpRequestHandler>();

        // Infinite loop to serve requests
        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            auto f = [socket](HttpRequestHandler* hrh) {hrh->handle_client(socket); };
            // Handle the client in a separate thread
            std::thread(f, socket).detach();
        }
    }
    catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}