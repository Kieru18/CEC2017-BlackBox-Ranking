#pragma once
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "DatabaseManager.h"
#include "JsonParser.hpp"
#include "ApiKeyManager.h"
#include "FunctionManager.h"
#include <boost/asio.hpp>
#include <Poco/Net/SecureSMTPClientSession.h>
#include <Poco/Net/MailMessage.h>
#include <Poco/Net/NetException.h>
#include <Poco/Net/StringPartSource.h>
#include <Poco/Net/FilePartSource.h>

using namespace Poco::Net;

class HttpRequestHandler {
private:
    std::unique_ptr<DatabaseManager> dbManager;
    std::unique_ptr<JsonParser> jsonParser;
    std::unique_ptr<ApiKeyManager> apiKeyManager;
    std::unique_ptr<FunctionManager> functionManager;

    std::string generateHTTPResponse(const std::string& textResponse);

public:
    HttpRequestHandler() 
        : dbManager(std::make_unique<DatabaseManager>()),
          jsonParser(std::make_unique<JsonParser>()),
          apiKeyManager(std::make_unique<ApiKeyManager>()),
          functionManager(std::make_unique<FunctionManager>()) {}

    void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
};
