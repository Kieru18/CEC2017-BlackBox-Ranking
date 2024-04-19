#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <iostream>
#include <memory>
#include <sstream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

class DatabaseManager {
public:
    std::unique_ptr<sql::ResultSet> getDatabaseResult(const std::string& request);
    std::string get_user_data(const int& id);
};

#endif // DATABASEMANAGER_H