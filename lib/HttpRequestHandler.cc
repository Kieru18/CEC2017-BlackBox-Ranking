#include <string>
#include <sstream>
#include <iostream>
#include <stdexcept>
#include <memory>

#include <boost/asio.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <nlohmann/json.hpp>

#include "DatabaseManager.h"
#include "JsonParser.hpp"
#include "ApiKeyManager.h"
#include "FunctionManager.h"
#include "MailManager.h"
#include "HttpRequestHandler.h"


void HttpRequestHandler::getMaxEvalCallLimit(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
    }   

    nlohmann::json data;
    file >> data;
    file.close();

    std::string gui_password;

    if (data.find("max_eval_call_limit") != data.end()) {
        max_eval_call_limit_ = data["max_eval_call_limit"];
    } 
    else 
    {
        throw std::runtime_error("Call limit credentials not found in JSON");
    }
}

void HttpRequestHandler::updateMaxEvalCallLimit(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Unable to open file" << std::endl;
        return;
    }

    nlohmann::json data;
    file >> data;
    file.close();

    // Aktualizacja wartości max_eval_call_limit
    data["max_eval_call_limit"] = max_eval_call_limit_;

    // Zapisanie zmodyfikowanych danych z powrotem do pliku
    std::ofstream outFile(path);
    if (!outFile.is_open()) {
        std::cerr << "Unable to open file for writing" << std::endl;
        return;
    }

    outFile << std::setw(4) << data << std::endl;
    outFile.close();
}


const std::string HttpRequestHandler::getGUIPassword(const std::string& path) {
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

const std::string HttpRequestHandler::generateHttpTextResponse(const std::string& text_response) {
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/plain\r\n"
                                "Content-Length: " + std::to_string(text_response.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                text_response;
    return response;
}

const std::string HttpRequestHandler::generateHttpHtmlResponse(const std::string& text_response) {
    const std::string response = "HTTP/1.1 200 OK\r\n"
                                "Content-Type: text/html\r\n"
                                "Content-Length: " + std::to_string(text_response.length()) + "\r\n"
                                "Connection: close\r\n\r\n" +
                                text_response;
    return response;
}


const std::string HttpRequestHandler::createWaitingUsersHTMLList(const std::string& credentials_path)
{
    std::stringstream result_stream;
    result_stream << R"(
        <div id="waiting_users_list" style="width: 600px; height: 400px; border: 1px solid #ccc; padding: 10px; overflow-y: scroll;">
        )";
    const std::vector<std::string> mails = dbManager->getUsersEmailsFromTable("request_table_name", credentials_path);
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


void HttpRequestHandler::handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path) {

    getMaxEvalCallLimit(credentials_path);

    std::stringstream error_stream;
    try {
                
        const std::string gui_password = getGUIPassword(credentials_path);
        const std::string hashed_gui_password = apiKeyManager->hashGivenString(gui_password);

        boost::asio::streambuf request(8192);
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
        std::size_t content_length = 0;

        while (std::getline(request_stream, line) && line != "\r") {
            std::istringstream header_line_stream(line);
            std::string header_name;
            header_line_stream >> header_name;
            if (header_name == "Content-Length:") {
                header_line_stream >> content_length;
            }
        }
        //std::cout << "Finished reading headers\n";

        // Read the body based on Content-Length
        std::string body;
        if (content_length > 0) {
            //std::cout << "Reading body of size " << content_length << "\n";
            std::vector<char> body_data(content_length);

            // Read any remaining part of the body from the initial request buffer
            std::size_t bytes_remaining = request.size();
            if (bytes_remaining > 0) {
                std::istreambuf_iterator<char> begin(&request), end;
                std::string remaining_body_part(begin, end);
                body += remaining_body_part;
            }

            // Read the rest of the body directly from the socket
            std::size_t bytes_to_read = content_length - body.size();
            if (bytes_to_read > 0) {
                boost::asio::mutable_buffers_1 buffer = boost::asio::buffer(body_data.data(), bytes_to_read);
                std::size_t bytes_read = socket->read_some(buffer, error);
                body.append(body_data.data(), bytes_read);
            }
        }

        //std::cout << "BODY\n" << body << "\n";

        std::string response;



        if (method == "POST" && api_path == "/registrate") {
            

            std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            
            if (!mailManager->isEmailValid(mail)){
                std::cout << "Użytkownik o niepoprawnym adresie mailowym " << mail <<" wyraża chęć rejestracji\n";

                response = generateHttpTextResponse("Niepoprawny adres email!\n");   
            } else {
                std::cout << "Użytkownik o adresie mailowym " << mail <<" wyraża chęć rejestracji\n";

                const bool is_user_already_registrated = dbManager->isUserRecordedInTable(mail, "users_table_name", credentials_path);
                const bool is_user_already_waiting_for_registration = dbManager->isUserRecordedInTable(mail, "request_table_name", credentials_path);
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
                    dbManager->addUserToRequestTable(mail, credentials_path);
                    response = generateHttpTextResponse("Nie istnieje użytkownik z takim adresem email, więc możesz założyć konto. Na podany adres e-mail dostaniesz klucz api (kiedyś).\n");
                }
            }
        }

        else if (method == "POST" && api_path == ("/"+hashed_gui_password+"/accept_user_registration"))
        {
            const std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            const std::string api_key = apiKeyManager->generateApiKey(20);
            const std::string hashed_api_key = apiKeyManager->hashGivenString(api_key);
            dbManager->deleteUserFromRequestTable(mail, credentials_path);
            dbManager->addUserToUsersTable(mail, hashed_api_key, credentials_path);
            try
            {
                mailManager->sendAcceptanceMail(mail, api_key, credentials_path);
            }
            catch (const std::exception& e){
                
                std::cout << "Możliwe, że wystąpił błąd przy wysyłaniu maila z potwierdzeniem i z kluczem API. Błąd: " << e.what() << "\n";
            }
            response = generateHttpTextResponse("Akceptujesz rejestrację użytkownika o mailu " + mail + "\n");

        }

        else if (method == "DELETE" && api_path == ("/"+hashed_gui_password+"/delete_from_request_table")) {

            const std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            dbManager->deleteUserFromRequestTable(mail, credentials_path);
            try
            {
                mailManager->sendRejectionMail(mail, credentials_path);
            }
            catch (const std::exception& e){
                std::cout << "Możliwe, że wystąpił błąd przy wysyłaniu maila z odmową. Błąd: " << e.what() << "\n";
            }
            std::cout << "Administrator usuwa z listy oczekujących na rejestrację użytkownika o mailu " << mail <<" (jeśli wyraża chęć rejestracji)\n";

            response = generateHttpTextResponse("Usuwasz z listy oczekujących na rejestrację użytkownika o mailu " + mail + " (jeśli wyraża chęć rejestracji)\n");
        }

        else if (method == "GET" && api_path == ("/"+hashed_gui_password+"/reset_spend")) {

            dbManager->resetSpendColumn(credentials_path);

            std::cout << "Administrator resetuje ilość zużytych wywołań funkcji dla wszystkich użytkowników\n";

            response = generateHttpTextResponse("Resetujesz ilość zużytych wywołań funkcji dla wszystkich użytkowników\n");
        }

        else if (method == "PUT" && api_path == ("/"+hashed_gui_password+"/change_max_eval_call_limit")) {
            const int new_limit = jsonParser->parseDataFromJson<int>(body, "new_limit");
            max_eval_call_limit_ = new_limit;
            updateMaxEvalCallLimit(credentials_path);
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
        
        else if (method == "GET" && api_path == "/"){
            std::stringstream html;
            html << R"(
                <!DOCTYPE html>
                <html lang="en">
                <head>
                    <meta charset="UTF-8">
                    <title>Strona admina Blackbox</title>
                    <script>
                        window.location.pathname = 'admin_page';
                    </script>
                </head>
                <body>
                </body>
                </html>)";
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
                <label id="max_eval_call_limit_label">Limit wywołań: )" << max_eval_call_limit_ << R"(</label>
                <div>
                <p>Ustaw nowy limit wywołań funkcji</p>
                <input type="number" id="max_eval_call_limit_input" name="max_eval_call_limit_input">
                <button type="button" id="max_eval_call_limit_button" onclick = set_max_eval_call_limit()>Zatwierdź nowy limit</button>
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
                    async function set_max_eval_call_limit()
                    {
                        let limit = document.getElementById("max_eval_call_limit_input").value;
                        let limit_num = parseInt(limit);

                        if (isNaN(limit_num)) {
                            alert("Nieprawidłowa wartość limitu wywołań funkcji. Wprowadź liczbę.");
                        }
                        else if (limit_num<0)
                        {
                            alert("Limit wywołań funkcji musi być nieujemny! Wprowadź nową wartość.");
                        } else {
                            const hashed_gui_password = ")" << hashed_gui_password << R"(";
                            const change_max_eval_call_limit_url = window.location.href.replace("/admin_page", "/"+hashed_gui_password+"/change_max_eval_call_limit");
                            try {
                                await fetch(change_max_eval_call_limit_url, {
                                    method: 'PUT',
                                    headers: {
                                        'Content-Type': 'application/json'
                                    },
                                    body: JSON.stringify({ new_limit: limit_num }),
                                });
                                document.getElementById("max_eval_call_limit_label").textContent = "Limit wywołań: " + limit_num.toString(); 
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

        else if(method == "POST" && api_path == "/verify_api_key") {
            const std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            const std::string api_key = jsonParser->parseDataFromJson<std::string>(body, "api_key");
            bool isUserRegistrated = dbManager->isUserRecordedInTable(mail, "users_table_name", credentials_path);
            bool isPasswordOk = false;
            if (isUserRegistrated){
                isPasswordOk = dbManager->isPasswordCorrect(mail, api_key, credentials_path);
            }
            if (isUserRegistrated && isPasswordOk) {
                response = generateHttpTextResponse("Mail i klucz API są poprawne");
            }
            else if(!isUserRegistrated) 
            {
                response = generateHttpTextResponse("Użytkownik o takim adresie email nie jest zarejestrowany");
            }
            else {
                response = generateHttpTextResponse("Klucz API jest niepoprawny");
            }
        }

        //@TODO verify if user is registered, check call limit, check if function exists
        else if (method == "POST" && api_path == "/evaluate") {
            const std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            std::cout << "Użytkownik o adresie email " << mail << " chce wywołać funkcję oceny\n";

            const std::string api_key = jsonParser->parseDataFromJson<std::string>(body, "api_key");
            bool isUserRegistrated = dbManager->isUserRecordedInTable(mail, "users_table_name", credentials_path);
            bool isPasswordOk = false;
            bool isEnoughToSpend = false;
            if (isUserRegistrated){
                isPasswordOk = dbManager->isPasswordCorrect(mail, api_key, credentials_path);
            }
            if (isUserRegistrated && isPasswordOk) {
                int alreadySpent = dbManager->getSpendParamOfUser(mail, credentials_path);
                isEnoughToSpend = (alreadySpent+1 <= max_eval_call_limit_); 
            }
            if ((isUserRegistrated && isPasswordOk) && isEnoughToSpend){
                const int function_number = jsonParser->parseDataFromJson<int>(body, "function_number");
                const std::vector<double> specimen = jsonParser->parseSpecimenFromJson(body);
                const int d = specimen.size();
                if (function_number<1 || function_number > 30)
                {
                    response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Funkcja może mieć numer od 1 do 30 (włącznie)!\"}");
                }
                
                else 
                {
                    if (((d == 2 || d == 10) || (d == 20 || d == 30)) || (d == 50 || d == 100)){
                        const double result = functionManager->getFunctionResults(function_number, specimen);
                        dbManager->incrementSpendParamForUser(mail, credentials_path);
                        response = generateHttpTextResponse("{\"is_valid\":true, \"result\":"+std::to_string(result)+"}");
                    }
                    else {
                        response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Osobnik może mieć wymiary 2, 10, 20, 30, 50 lub 100\"}");
                    }
                }
            }
            else if(!isUserRegistrated){
                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Użytkownik nie jest zarejestrowany\"}");
            }
            else if(!isPasswordOk){
                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Podany klucz API jest niepoprawny\"}");
            }
            else if(!isEnoughToSpend){
                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Osiągnięto limit wywołań funkcji oceny\"}");
            }
        }

        else if (method == "POST" && api_path == "/evaluate_population") {
            
            const std::string mail = jsonParser->parseDataFromJson<std::string>(body, "mail");
            std::cout << "Użytkownik o adresie email " << mail << "chce wywołać funkcję oceny\n";

            const std::string api_key = jsonParser->parseDataFromJson<std::string>(body, "api_key");
            bool isUserRegistrated = dbManager->isUserRecordedInTable(mail, "users_table_name", credentials_path);
            bool isPasswordOk = false;
            if (isUserRegistrated){
                isPasswordOk = dbManager->isPasswordCorrect(mail, api_key, credentials_path);
            }

            if (isUserRegistrated && isPasswordOk){
                const int function_number = jsonParser->parseDataFromJson<int>(body, "function_number");
                if (function_number<1 || function_number > 30)
                {
                    response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Funkcja może mieć numer od 1 do 30 (włącznie)!\"}");
                }
                else 
                {
                    const std::vector<std::vector<double>> population = jsonParser->parsePopulationFromJson(body);
                    if (population.size() == 0)
                    {
                        response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Wektor osobników nie może być pusty\"}");
                    }
                    else{
                        int alreadySpent = dbManager->getSpendParamOfUser(mail, credentials_path);
                        int size_of_population = population.size();
                        if(size_of_population + alreadySpent <= max_eval_call_limit_) {
                            bool are_dim_of_specimen_ok = true;
                            for(int i = 0; i<size_of_population; ++i){
                                const int d = population[i].size();
                                if (!(d == 2 || d == 10 || d == 20 || d == 30 || d == 50 || d == 100))
                                {
                                    are_dim_of_specimen_ok = false;
                                    break;
                                }
                            }
                            if(!are_dim_of_specimen_ok) {
                                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Osobnik może mieć wymiary 2, 10, 20, 30, 50 lub 100\"}");
                            }
                            else {
                                //TODO wypisz wyniki dla populacji

                                const std::vector<double> results = functionManager->getFunctionResults(function_number, population);
                                dbManager->increaseSpendParamForUser(mail, population.size(), credentials_path);

                                std::string results_string = "[";
                                for (const double result : results) {
                                    results_string += std::to_string(result) + ", ";
                                }
                                results_string = results_string.substr(0, results_string.size() - 2);
                                results_string += "]";
                                response = generateHttpTextResponse("{\"is_valid\":true, \"result\":"+results_string+"}");
                            }
                        }
                        else {
                            response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Za duża populacja, zaszłoby przekroczenie limitu wywołań funkcji oceny\"}");
                        }
                    }
                }
            }
            else if(!isUserRegistrated){
                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Użytkownik nie jest zarejestrowany\"}");
            }
            else if(!isPasswordOk){
                response = generateHttpHtmlResponse("{\"is_valid\":false,\"error\": \"Podany klucz API jest niepoprawny\"}");
            }

            

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
