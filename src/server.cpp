#include "server.h"
#include "db.h"
#include "hash.h"

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "libs/json.hpp"

const std::string getGUIPassword(const std::string& path){
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
    }   

    nlohmann::json data;
    file >> data;
    file.close();

    std::string gui_password;

    if (data.find("gui") != data.end()) {
        auto gui_credentials = data["gui"];

        if (gui_credentials.find("password") != gui_credentials.end()) {
            gui_password = gui_credentials["password"];
        }
        else{
            throw std::runtime_error("GUI password not found in credentials");
        }
    } 
    else 
    {
        throw std::runtime_error("GUI credentials not found in JSON");
    }
    return gui_password;
}



template<typename T>
T parseDataFromJson(const std::string& json_data, const std::string& child_key){
    T result;
    std::stringstream error_stream;
    try {
        boost::property_tree::ptree root;
        std::istringstream json_stream(json_data);
        boost::property_tree::read_json(json_stream, root);
        result = root.get_child(child_key).get_value<T>();
        
    } catch (const std::exception& e) {
        error_stream << "Error parsing JSON: " << e.what();
        throw std::runtime_error(error_stream.str());
    }

    return result;
}

const std::string generateHttpTextResponse(const std::string& text_response){
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: " + std::to_string(text_response.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                text_response;
    return response;
}
const std::string generateHttpHtmlResponse(const std::string& text_response){
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "Content-Length: " + std::to_string(text_response.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                text_response;
    return response;
}



void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path) {

    std::stringstream error_stream;
    try {
        
        const std::string gui_password = getGUIPassword(credentials_path);
        const std::string hashed_gui_password = hashGivenString(gui_password);

        boost::asio::streambuf request;
        boost::system::error_code error;
        boost::asio::read_until(*socket, request, "\r\n\r\n", error);

        if (error && error != boost::asio::error::eof) {
            throw boost::system::system_error(error);
        }

        std::istream request_stream(&request);
        std::string request_line, method, api_path;
        std::getline(request_stream, request_line);
        std::istringstream request_line_stream(request_line);
        request_line_stream >> method >> api_path;

        std::string line;

        while (std::getline(request_stream, line) && line != "\r") {}

        std::string body(std::istreambuf_iterator<char>(request_stream), {});

        std::string response;

        if (method == "POST" && api_path == "/registrate") {
            std::string mail = parseDataFromJson<std::string>(body, "mail");
            std::cout << "Użytkownik o adresie mailowym " << mail <<" wyraża chęć rejestracji\n";

            const bool is_user_already_registrated = isUserRecordedInTable(mail, "users_table_name", credentials_path);
            const bool is_user_already_waiting_for_registration = isUserRecordedInTable(mail, "request_table_name", credentials_path);
            if (is_user_already_registrated)
            {
                response = generateHttpTextResponse("Istnieje użytkownik z takim adresem email, nie możesz założyć konta.\n");
            }
            else if (is_user_already_waiting_for_registration)
            {
                response = generateHttpTextResponse("Użytkownik z takim adresem email nadal czeka na akceptację.\n");
            }
            else
            {
                addUserToRequestTable(mail, credentials_path);
                response = generateHttpTextResponse("Nie istnieje użytkownik z takim adresem email, więc możesz założyć konto. Na podany adres e-mail dostaniesz klucz api (kiedyś).\n");

            }
        }

        else if (method == "DELETE" && api_path == ("/"+hashed_gui_password+"/delete_from_request_table")) {
            std::string mail = parseDataFromJson<std::string>(body, "mail");

            deleteUserFromRequestTable(mail, credentials_path);

            std::cout << "Administrator usuwa z listy oczekujących na rejestrację użytkownika o mailu " << mail <<" (jeśli wyraża chęć rejestracji)\n";

            response = generateHttpTextResponse("Usuwasz z listy oczekujących na rejestrację użytkownika o mailu " + mail + " (jeśli wyraża chęć rejestracji)\n");
        }

        else if (method == "GET" && api_path == ("/"+hashed_gui_password+"/reset_spend")) {

            resetSpendColumn(credentials_path);

            std::cout << "Administrator resetuje ilość zużytych wywołań funkcji dla wszystkich użytkowników\n";

            response = generateHttpTextResponse("Resetujesz ilość zużytych wywołań funkcji dla wszystkich użytkowników\n");
        }

        else if (method == "GET" && api_path == "/admin_login_page"){
            std::stringstream html;
            html << R"(
                <!DOCTYPE html>
                <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <title>Logowanie do BlackBox</title>

                </head>
                <body>
                <h1>Zaloguj się</h1>
                <label for='password'>Hasło</label>
                <input type='password' id='password' name='password'>
                <button type='button' id='send_password' onclick='send_password_fun()'>Zatwierdź</button>
                <script>
                    function send_password_fun() {
                        const real_password = ")" << gui_password << R"("; 
                        const password_input = document.getElementById("password");
                        const password = password_input.value;
                        localStorage.gui_password = password;
                        console.log("podane haslo: ", localStorage.gui_password);
                        if (password === "")
                        {
                            alert("Nie podano hasła");
                        }
                        else if (password === real_password) {
                            window.location.pathname = 'admin_page';
                        }
                        else {
                            alert("Podano złe hasło");
                        }
                    }
                </script>
                </body></html>
            )";
            response = generateHttpHtmlResponse(html.str());
        }

        else if (method == "GET" && api_path == "/admin_page"){
            std::stringstream html;
            html << R"(
                <!DOCTYPE html>
                <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <title>Strona admina Blackbox</title>
                    <script>
                        const real_password = ")" << gui_password << R"(";
                        if (!localStorage.gui_password || localStorage.gui_password !== real_password){
                            window.location.pathname = 'admin_login_page';
                        }
                    </script>
                </head>
                <body>
                <h1>Ściśle tajne</h1>
                <div style="display: flex; flex-direction: column; width: 25%; float: left;">
                <button id="log_out_button" onclick = log_out() ">Wyloguj się</button>
                <button id="reset_spend_button" onclick = reset_spend() ">Resetuj wywołania funkcji dla wszystkich</button>
                </div>
                <script>
                    function log_out()
                    {
                        localStorage.removeItem('gui_password');
                        window.location.pathname = 'admin_login_page';
                    }
                    async function reset_spend()
                    {
                        const hashed_gui_password = ")" << hashed_gui_password << R"(";
                        const reset_spend_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/reset_spend");
                        console.log(reset_spend_url);
                            try {
                            const response = await fetch(reset_spend_url, {
                                method: 'GET',
                            });
                            } catch (e) {
                                console.error('Fetch error:', e);
                            }
                    }

                </script>
                </body></html>
            )";
            response = generateHttpHtmlResponse(html.str());
        }

        else {
            response = generateHttpTextResponse("404 Not Found");
        }

        boost::asio::write(*socket, boost::asio::buffer(response), error);
        if (error) {
            throw boost::system::system_error(error);
        }

    } catch (std::exception& e) {
        error_stream << "Exception in thread: " << e.what();
        throw std::runtime_error(error_stream.str());

    }
}
