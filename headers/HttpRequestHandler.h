#pragma once
#include <iostream>
#include <sstream>
#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include "DatabaseManager.h"
#include "JsonParser.h"
#include "ApiKeyManager.h"
#include "FunctionManager.h"

#include <memory>

class HttpRequestHandler {
private:
    DatabaseManager dbManager;
    JsonParser jsonParser;
    ApiKeyManager apiKeyManager;

    std::string generateHTTPResponse(const std::string& textResponse);

public:
    HttpRequestHandler();

    void handle_client(std::shared_ptr<boost::asio::ip::tcp::socket> socket);
};
