#pragma once
#include <iostream>
#include <memory>
#include <sstream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

class DatabaseManager {
public:
    DatabaseManager() = default;
    ~DatabaseManager() = default;
    std::unique_ptr<sql::ResultSet> getDatabaseResult(const std::string& request);
    std::string getUserData(const int& id);
};
