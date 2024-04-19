#include <boost/asio.hpp>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <sstream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include <ctime>

#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/FilePartSource.h>

#include <random>
#include <string>

#include "headers/HttpRequestHandler.h"
#include "headers/ApiKeyManager.h"
#include "headers/DatabaseManager.h"
#include "headers/JsonParser.h"
#include "headers/FunctionManager.h"

using boost::asio::ip::tcp;

using namespace Poco::Net;


// Main function
int main() {
    try {
        boost::asio::io_context io_context;

        // Setting up the server to listen on TCP port 8080
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 8080));
        std::cout << "Server is listening on port 8080...\n";

        std::unique_ptr<HttpRequestHandler> httpHandler = std::make_unique<HttpRequestHandler>();

        // Infinite loop to serve requests
        while (true) {
            auto socket = std::make_shared<tcp::socket>(io_context);
            acceptor.accept(*socket);

            // Handle the client in a separate thread
            std::thread([&httpHandler, socket]() {
                httpHandler->handle_client(socket);
            }).detach();
        }
    } catch (std::exception& e) {
        std::cerr << "Exception:" << e.what() << "\n";
    }

    return 0;
}