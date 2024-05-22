#ifndef DB_H
#define DB_H

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include <string>
#include <tuple>

const std::tuple<std::string, std::string, std::string, std::string, std::string> getDatabaseCredentials(const std::string& path);
const std::string getTableName(const std::string& path);
sql::ResultSet& getDatabaseResult(const std::string& request, const std::string& path);
bool isUserRegistratedInDatabase(const std::string& mail_address, const std::string& path);


#endif