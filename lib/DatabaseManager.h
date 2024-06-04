#pragma once

#include <string>
#include <tuple>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>
#include "ApiKeyManager.h"

class DatabaseManager {
private:
    std::unique_ptr<ApiKeyManager> apiKeyManager;
    const static std::unordered_map<std::string, int> function_name_to_case;
public:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    const std::tuple<std::string, std::string, std::string, std::string> getDatabaseCredentials(const std::string& path);
    const std::string getTableName(const std::string& table_type, const std::string& path);
    sql::ResultSet& getDatabaseResult(const std::string& request, const std::string& path);
    void makeDatabaseAction(const std::string& request, const std::string& path);
    bool isUserRecordedInTable(const std::string& mail_address, const std::string& table_type, const std::string& path);
    void addUserToRequestTable(const std::string& mail_address, const std::string& path);
    void addUserToUsersTable(const std::string& mail_address, const std::string& hashed_api_key, const std::string& path);
    void deleteUserFromRequestTable(const std::string& mail_address, const std::string& path);
    void resetSpendColumn(const std::string& path);
    std::vector<std::string> getUsersEmailsFromTable(const std::string& table_type, const std::string& path);
    void incrementSpendParamForUser(const std::string& mail_address, const std::string& path);
    void increaseSpendParamForUser(const std::string& mail_address, const int addition, const std::string& path);
    bool isPasswordCorrect(const std::string& mail_address, const std::string& given_password, const std::string& path);
    int getSpendParamOfUser(const std::string& mail_address, const std::string& path);
};
