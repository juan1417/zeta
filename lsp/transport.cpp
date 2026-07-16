#include "transport.hpp"
#include <iostream>
#include <sstream>

namespace zeta_lsp {

json Transport::read_message() {
    std::string line;
    int content_length = -1;

    while (true) {
        if (!std::getline(std::cin, line)) {
            throw std::runtime_error("EOF");
        }
        if (line.empty() || line == "\r") break;
        if (line.size() >= 16 && line.substr(0, 16) == "Content-Length: ") {
            content_length = std::stoi(line.substr(15));
        }
    }

    if (content_length < 0) {
        throw std::runtime_error("No Content-Length header");
    }

    std::string body(content_length, '\0');
    std::cin.read(&body[0], content_length);
    if (std::cin.gcount() != content_length) {
        throw std::runtime_error("Incomplete body");
    }
    return json::parse(body);
}

void Transport::write_message(const json& msg) {
    std::string body = msg.dump();
    std::cout << "Content-Length: " << body.size() << "\r\n\r\n" << body;
    std::cout.flush();
}

} // namespace zeta_lsp
