#include "db.h"

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

#include "libs/json.hpp"

#include <string>
#include <tuple>
#include <iostream>
#include <sstream>
#include <fstream>
#include <stdexcept>

const std::tuple<std::string, std::string, std::string, std::string> getDatabaseCredentials(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
    }   

    nlohmann::json data;
    file >> data;
    file.close();

    std::string conn_address;
    std::string username;
    std::string password;
    std::string db_name;

    if (data.find("database") != data.end()) {
        auto database_credentials = data["database"];

        if (database_credentials.find("conn_address") != database_credentials.end()) {
            conn_address = database_credentials["conn_address"];
        }

        else{
            throw std::runtime_error("Connection address not found in credentials");
        }

        if (database_credentials.find("username") != database_credentials.end()) {
            username = database_credentials["username"];
        }
        else{
            throw std::runtime_error("Username of database not found in credentials");
        }

        if (database_credentials.find("password") != database_credentials.end()) {
            password = database_credentials["password"];
        }
        else{
            throw std::runtime_error("Database password not found in credentials");
        }

        if (database_credentials.find("db_name") != database_credentials.end()) {
            db_name = database_credentials["db_name"];
        }
        else{
            throw std::runtime_error("Database name not found in credentials");
        }
    } 
    else 
    {
        throw std::runtime_error("Database credentials not found in JSON");
    }
    return std::make_tuple(conn_address, username, password, db_name);
}

const std::string getTableName(const std::string& table_type, const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
    }   

    nlohmann::json data;
    file >> data;
    file.close();

    std::string table_name;

    if (data.find("database") != data.end()) {
        auto database_credentials = data["database"];

        if (database_credentials.find(table_type) != database_credentials.end()) {
            table_name = database_credentials[table_type];
        }
        else{
            throw std::runtime_error("Searched table name not found in credentials");
        }
    } 
    else 
    {
        throw std::runtime_error("Database credentials not found in JSON");
    }
    return table_name;
}

sql::ResultSet& getDatabaseResult(const std::string& request, const std::string& path){

    const std::tuple<std::string, std::string, std::string, std::string> credentials = getDatabaseCredentials(path);
    const std::string conn_address = std::get<0>(credentials);
    const std::string username = std::get<1>(credentials);
    const std::string password = std::get<2>(credentials);
    const std::string db_name = std::get<3>(credentials);

    sql::Driver *driver;
    sql::Connection *connection;
    sql::Statement *statement;

    try {
        driver = get_driver_instance();
        connection = driver->connect(conn_address, username, password);
        connection->setSchema(db_name);
        statement = connection->createStatement();
        return *(statement->executeQuery(request));
    }

    catch (const std::exception &e) {
        std::stringstream error_stream;
        error_stream << "SQL Error: " << e.what();
        throw std::runtime_error(error_stream.str());
    }
}

void makeDatabaseAction(const std::string& request, const std::string& path){
    const std::tuple<std::string, std::string, std::string, std::string> credentials = getDatabaseCredentials(path);
    const std::string conn_address = std::get<0>(credentials);
    const std::string username = std::get<1>(credentials);
    const std::string password = std::get<2>(credentials);
    const std::string db_name = std::get<3>(credentials);

    sql::Driver *driver;
    sql::Connection *connection;
    sql::Statement *statement;

    try {
        driver = get_driver_instance();
        connection = driver->connect(conn_address, username, password);
        connection->setSchema(db_name);
        statement = connection->createStatement();
        statement->execute(request);
        delete statement;
        delete connection;
    }

    catch (const sql::SQLException &e) {
        std::stringstream error_stream;
        error_stream << "SQL Error: " << e.what() << ". MySQL error code: " << e.getErrorCode() << ", SQLState: " << e.getSQLState();
        throw std::runtime_error(error_stream.str());
    }
    catch (const std::exception &e) {
        std::stringstream error_stream;
        error_stream << "Error: " << e.what();
        throw std::runtime_error(error_stream.str());
    }
}

bool isUserRecordedInTable(const std::string& mail_address, const std::string& table_type, const std::string& path){
    
    std::stringstream error_stream;
    const std::string table_name = getTableName(table_type, path);
    std::stringstream ss_users_table_request;
    ss_users_table_request <<"SELECT COUNT(*) AS users_count FROM " << table_name << " WHERE mail = " << "'" << mail_address <<"';";
    sql::ResultSet& db_response = getDatabaseResult(ss_users_table_request.str(), path);
    try {
        if (db_response.next()) {
            int num_of_found_users = db_response.getInt("users_count");
            if (num_of_found_users == 0){
                return false;
            }
            else if (num_of_found_users == 1)
            {
                return true;
            }
            else {
                error_stream << "There's not zero, and not only one user with the mail in the table"<<mail_address;
                throw std::runtime_error(error_stream.str());
            }
        }
        else {
            error_stream << "Error with response from SQL for function isUserRegistratedInDatabase. Requested mail: "<<mail_address;
            throw std::runtime_error(error_stream.str());
        }
    }
    catch (const std::exception &e) {
        throw std::runtime_error(e.what());
    }
}

void addUserToRequestTable(const std::string& mail_address, const std::string& path)
{
    const std::string request_table_name = getTableName("request_table_name", path);
    std::stringstream ss_request_table_request;
    ss_request_table_request <<"INSERT INTO " << request_table_name << " VALUES " << "('" << mail_address <<"');";
    makeDatabaseAction(ss_request_table_request.str(), path);
}

void deleteUserFromRequestTable(const std::string& mail_address, const std::string& path)
{
    const std::string request_table_name = getTableName("request_table_name", path);
    std::stringstream ss_request_table_request;
    ss_request_table_request <<"DELETE FROM " << request_table_name << " WHERE mail = '" << mail_address <<"';";
    makeDatabaseAction(ss_request_table_request.str(), path);
}

void resetSpendColumn(const std::string& path){
    const std::string users_table_name = getTableName("users_table_name", path);
    std::stringstream ss_users_table_request;
    ss_users_table_request <<"UPDATE " << users_table_name << " SET spend = 0;";
    makeDatabaseAction(ss_users_table_request.str(), path);
}