#include "server.h"
#include "db.h"
#include "hash.h"
#include "mail.h"

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>
#include "libs/json.hpp"

int CALL_LIMIT = 0;

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


const std::string createWaitingUsersHTMLList(const std::string& credentials_path)
{
    std::stringstream result_stream;
    result_stream << R"(
        <div id="waiting_users_list" style="width: 600px; height: 400px; border: 1px solid #ccc; padding: 10px; overflow-y: scroll;">
        )";
    const std::vector<std::string> mails = getUsersEmailsFromTable("request_table_name", credentials_path);
    for(std::string mail : mails)
    {
        result_stream << R"(
            <div style="display: flex; justify-content: space-around; align-items: center; gap: 10px;">
                <button style="display: block; width: 100%; margin-bottom: 5px; margin-top: 5px;" onclick = registrate_waiting_user(')"<<mail<<R"(')>Zaakceptuj )"<<mail<<R"(</button>
                <button style="display: block; width: 100%; margin-bottom: 5px; margin-top: 5px;" onclick = delete_waiting_user(')"<<mail<<R"(')>Usuń )"<<mail<<R"(</button>
            </div>
            )";
    }
    result_stream << R"(
        </div>
    )";
    return result_stream.str();
}


void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path) {
    //std::cout<<CALL_LIMIT<<"\n";
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
            
            if (!isEmailValid(mail)){
                std::cout << "Użytkownik o niepoprawnym adresie mailowym " << mail <<" wyraża chęć rejestracji\n";

                response = generateHttpTextResponse("Niepoprawny adres email!\n");   
            } else {
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
        }

        else if (method == "POST" && api_path == ("/"+hashed_gui_password+"/accept_user_registration"))
        {
            const std::string mail = parseDataFromJson<std::string>(body, "mail");
            const std::string api_key = generateApiKey(20);
            const std::string hashed_api_key = hashGivenString(api_key);
            deleteUserFromRequestTable(mail, credentials_path);
            addUserToUsersTable(mail, hashed_api_key, credentials_path);
            try
            {
                sendAcceptanceMail(mail, api_key, credentials_path);
            }
            catch (const std::exception& e){
                
                std::cout << "Możliwe, że wystąpił błąd przy wysyłaniu maila z potwierdzeniem i z kluczem API. Błąd: " << e.what() << "\n";
            }
            response = generateHttpTextResponse("Akceptujesz rejestrację użytkownika o mailu " + mail + "\n");

        }

        else if (method == "DELETE" && api_path == ("/"+hashed_gui_password+"/delete_from_request_table")) {

            const std::string mail = parseDataFromJson<std::string>(body, "mail");
            deleteUserFromRequestTable(mail, credentials_path);
            try
            {
                sendRejectionMail(mail, credentials_path);
            }
            catch (const std::exception& e){
                std::cout << "Możliwe, że wystąpił błąd przy wysyłaniu maila z odmową. Błąd: " << e.what() << "\n";
            }
            std::cout << "Administrator usuwa z listy oczekujących na rejestrację użytkownika o mailu " << mail <<" (jeśli wyraża chęć rejestracji)\n";

            response = generateHttpTextResponse("Usuwasz z listy oczekujących na rejestrację użytkownika o mailu " + mail + " (jeśli wyraża chęć rejestracji)\n");
        }

        else if (method == "GET" && api_path == ("/"+hashed_gui_password+"/reset_spend")) {

            resetSpendColumn(credentials_path);

            std::cout << "Administrator resetuje ilość zużytych wywołań funkcji dla wszystkich użytkowników\n";

            response = generateHttpTextResponse("Resetujesz ilość zużytych wywołań funkcji dla wszystkich użytkowników\n");
        }

        else if (method == "PUT" && api_path == ("/"+hashed_gui_password+"/change_call_limit")) {
            const int new_limit = parseDataFromJson<int>(body, "new_limit");
            CALL_LIMIT = new_limit;
            response = generateHttpTextResponse("Zmieniono limit wywołań funkcji na: " + std::to_string(new_limit) + "\n");
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
                <button type='button' id='send_password' onclick = send_password_fun()>Zatwierdź</button>
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
                <button id="log_out_button" onclick = log_out()>Wyloguj się</button>
                <label id="call_limit_label">Limit wywołań: 0</label>
                <div>
                <p>Ustaw nowy limit wywołań funkcji</p>
                <input type="number" id="call_limit_input" name="call_limit_input">
                <button type="button" id="call_limit_button" onclick = set_call_limit()>Zatwierdź nowy limit</button>
                </div>
                <button id="reset_spend_button" onclick = reset_spend()>Resetuj wywołania funkcji dla wszystkich zarejestrowanych użytkowników</button>
                <h2>Lista kandydatów chcących wziąć udział w konkursie</h2>
                )" << createWaitingUsersHTMLList(credentials_path) << R"(
                </div>
                <script>
                    function log_out()
                    {
                        localStorage.removeItem('gui_password');
                        window.location.pathname = 'admin_login_page';
                    }
                    async function set_call_limit()
                    {
                        let limit = document.getElementById("call_limit_input").value;
                        let limit_num = parseInt(limit);

                        if (isNaN(limit_num)) {
                            alert("Nieprawidłowa wartość limitu wywołań funkcji. Wprowadź liczbę.");
                        }
                        else if (limit_num<0)
                        {
                            alert("Limit wywołań funkcji musi być nieujemny! Wprowadź nową wartość.");
                        } else {
                            const hashed_gui_password = ")" << hashed_gui_password << R"(";
                            const change_call_limit_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/change_call_limit");
                            try {
                                await fetch(change_call_limit_url, {
                                    method: 'PUT',
                                    headers: {
                                        'Content-Type': 'application/json'
                                    },
                                    body: JSON.stringify({ new_limit: limit_num }),
                                });
                                document.getElementById("call_limit_label").textContent = "Limit wywołań: " + limit_num.toString(); 
                            } catch (e) {
                            console.error('Fetch error:', e);
                            }
                        }
                    }
                    async function registrate_waiting_user(mail){
                        const hashed_gui_password = ")" << hashed_gui_password << R"(";
                        const registrate_waiting_user_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/accept_user_registration");
                        try {
                            await fetch(registrate_waiting_user_url, {
                                method: 'POST',
                                headers: {
                                    'Content-Type': 'application/json'
                                },
                                body: JSON.stringify({ mail: mail }),
                            });
                            location.reload();
                        } catch (e) {
                            console.error('Fetch error:', e);
                        }
                        
                    }
                    async function delete_waiting_user(mail)
                    {
                        const hashed_gui_password = ")" << hashed_gui_password << R"(";
                        const delete_waiting_user_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/delete_from_request_table");
                        try {
                            await fetch(delete_waiting_user_url, {
                                method: 'DELETE',
                                headers: {
                                    'Content-Type': 'application/json'
                                },
                                body: JSON.stringify({ mail: mail }),
                            });
                            location.reload();
                        } catch (e) {
                            console.error('Fetch error:', e);
                        }
                    }
                    async function reset_spend()
                    {
                        const hashed_gui_password = ")" << hashed_gui_password << R"(";
                        const reset_spend_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/reset_spend");
                        try {
                            await fetch(reset_spend_url, {
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
