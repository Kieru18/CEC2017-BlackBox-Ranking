#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>

#include "DatabaseManager.h"
#include "JsonParser.hpp"
#include "ApiKeyManager.h"
#include "FunctionManager.h"
#include "MailManager.h"


class HttpRequestHandler {
private:
    std::unique_ptr<DatabaseManager> dbManager;
    std::unique_ptr<JsonParser> jsonParser;
    std::unique_ptr<ApiKeyManager> apiKeyManager;
    std::unique_ptr<FunctionManager> functionManager;
    std::unique_ptr<MailManager> mailManager;
    int max_eval_call_limit_;

    const std::string generateHttpTextResponse(const std::string& text_response);
    const std::string generateHttpHtmlResponse(const std::string& text_response);

    void getMaxEvalCallLimit(const std::string& path);
    void updateMaxEvalCallLimit(const std::string& path);

    const std::string getGUIPassword(const std::string& path);
    const std::string createWaitingUsersHTMLList(const std::string& credentials_path);


public:
    HttpRequestHandler() 
        : dbManager(std::make_unique<DatabaseManager>()),
          jsonParser(std::make_unique<JsonParser>()),
          apiKeyManager(std::make_unique<ApiKeyManager>()),
          functionManager(std::make_unique<FunctionManager>()),
          max_eval_call_limit_(0) {}

    void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path);
};
