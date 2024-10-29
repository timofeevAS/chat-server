// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "daemon/daemon.h"
#include "logger/logger.h"
#include "server/server.h"

int main(int argc, char const *argv[])
{
    const std::string appName = argv[0];

    // Initialize logger with the application name
    Logger::init(appName);
    Logger::info("Starting chat server application", "main()");

    // Start the process as a daemon
    Daemon::start();

    // Initialize and run the chat server on the specified port
    int port = 8008;
    ChatServer server(port);
    server.run();

    // Close the logger
    Logger::close();

    return 0;
}