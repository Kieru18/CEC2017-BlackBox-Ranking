#pragma once

#include <string>
#include <tuple>

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>


class DatabaseManager {
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
};
