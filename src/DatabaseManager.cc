#include <iostream>
#include <memory>
#include <sstream>
#include <mysql_driver.h>
#include <mysql_connection.h>
#include <cppconn/statement.h>
#include <cppconn/resultset.h>

class DatabaseManager {
public:
    std::unique_ptr<sql::ResultSet> getDatabaseResult(const std::string& request){
        sql::Driver *driver;
        sql::Connection *connection;
        sql::Statement *statement;
        try {
            // Create a MySQL driver instance
            driver = get_driver_instance();
            // Establish a connection to the MySQL database
            connection = driver->connect("tcp://127.0.0.1:3306", "zpr_user", "zpr_password");
            // Use the specific database
            connection->setSchema("zpr_example_database");
            // Execute SQL query
            statement = connection->createStatement();
            return std::unique_ptr<sql::ResultSet>(statement->executeQuery(request));
        }
        catch (sql::SQLException &e) {
            std::cerr << "SQL Error: " << e.what() << std::endl;
        }
        return nullptr;
    } 

    std::string get_user_data(const int& id){
        try {
            std::stringstream ssreq;
            ssreq << "SELECT * FROM zpr_example_table where id = " << id;
            std::unique_ptr<sql::ResultSet> database_request_result = getDatabaseResult(ssreq.str());
            if (database_request_result && database_request_result->next()) {
                // Pobieranie danych z wyniku zapytania
                int fetched_id = database_request_result->getInt("id"); // Można użyć getInt z nazwą kolumny
                std::string name = database_request_result->getString("name");
                // Tworzenie wynikowego stringa z danymi użytkownika
                std::stringstream ssres;
                ssres << "ID: " << fetched_id << " name: " << name;
                std::string user_data_result = ssres.str();
                return user_data_result;
            } else {
                // Jeśli zapytanie nie zwróciło żadnych rekordów
                return "ERROR - No user data found for ID: " + std::to_string(id);
            }
        } catch (sql::SQLException &e) {
            std::cerr << "SQL Error: " << e.what() << std::endl;
            return "ERROR - SQL Exception occurred";
        }
    }
};
