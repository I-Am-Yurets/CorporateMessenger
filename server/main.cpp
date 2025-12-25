#include <iostream>
#include <boost/asio.hpp>
#include "Server.h"

const short DEFAULT_PORT = 12345;

int main(int argc, char* argv[]) {
    try {
        short port = DEFAULT_PORT;

        if (argc > 1) {
            port = static_cast<short>(std::atoi(argv[1]));
        }

        std::cout << "==================================" << std::endl;
        std::cout << " Corporate Messenger Server" << std::endl;
        std::cout << "==================================" << std::endl;
        std::cout << "Starting server on port " << port << "..." << std::endl;

        boost::asio::io_context ioContext;
        Server server(ioContext, port);

        std::cout << "Server is running. Press Ctrl+C to stop." << std::endl;
        std::cout << "==================================" << std::endl;

        ioContext.run();

    } catch (std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}