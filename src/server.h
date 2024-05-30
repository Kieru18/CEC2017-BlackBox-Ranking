#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <boost/asio.hpp>

void getCallLimit(const std::string& path);
void updateCallLimit(const std::string& path);
template<typename T>
T parseDataFromJson(const std::string& json_data, const std::string& child_key);

const std::string generateHttpTextResponse(const std::string& text_response);
const std::string generateHttpHtmlResponse(const std::string& text_response);

void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path);
const std::string getGUIPassword(const std::string& path);
#endif