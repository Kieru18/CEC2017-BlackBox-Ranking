#ifndef SERVER_H
#define SERVER_H

#include <string>
#include <memory>
#include <boost/asio.hpp>


template<typename T>
T parseDataFromJson(const std::string& json_data, const std::string& child_key);

const std::string generateHttpResponse(const std::string& text_response);

void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket, const std::string& credentials_path);
#endif