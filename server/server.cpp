// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "server.h"

#include "logger/logger.h"
#include "pch.h"


ChatServer::ChatServer(int port)
{
    setupServerSocket(port); // Initialize the server socket
    FD_ZERO(&master_set);    // Initialize the master file descriptor set
    FD_SET(server_fd, &master_set); // Add the server_fd to the master set
    max_fd = server_fd;      // Set the initial maximum file descriptor
}

void ChatServer::setupServerSocket(int port)
{
    const std::string methodName = "ChatServer::setupServerSocket";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        Logger::errorWithErrno("Failed to create server socket.", methodName);
        exit(EXIT_FAILURE);
    }

    fcntl(server_fd, F_SETFL, O_NONBLOCK); // Set the server socket to non-blocking mode

    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        Logger::errorWithErrno("Failed to bind server socket.", methodName);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, 10) < 0)
    {
        Logger::errorWithErrno("Failed to listen on server socket.", methodName);
        exit(EXIT_FAILURE);
    }

    Logger::info("Server socket setup complete.", methodName);
}

void ChatServer::run()
{
    const std::string methodName = "ChatServer::run()";
    Logger::info("Chat server started and awaiting connections", methodName);

    while (true)
    {
        fd_set read_fds = master_set;  // Copy the master set to read_fds for select
        timeval timeout{15, 0};        // Timeout for select

        int activity = select(max_fd + 1, &read_fds, NULL, NULL, &timeout);

        if (activity < 0 && errno != EINTR)
        {
            Logger::errorWithErrno("Select error", methodName);
            break;
        }

        // Check for new connections on the server socket
        if (FD_ISSET(server_fd, &read_fds))
        {
            handleNewConnection();
        }

        // Iterate through clients and handle incoming messages
        for (auto it = clients.begin(); it != clients.end();)
        {
            int client_fd = it->first;
            if (FD_ISSET(client_fd, &read_fds))
            {
                handleClientMessage(client_fd);
            }
            ++it;
        }
    }
}

void ChatServer::askForNickname(int client_fd)
{
    std::string message = "Please enter your nickname: ";
    send(client_fd, message.c_str(), message.size(), 0); // Request nickname from the new client
}

void ChatServer::setNickname(int client_fd, const std::string& nickname)
{
    const std::string methodName = "ChatServer::setNickname()";
    clients[client_fd] = nickname; // Associate the nickname with the client socket

    std::stringstream ss;
    ss << "Client " << std::to_string(client_fd) << " set nickname to " << nickname;

    Logger::info(ss.str(), methodName);

    // Notify other clients that a new user has joined
    std::string join_message = nickname + " has joined the chat!";
    broadcastMessage(join_message, client_fd);
}

void ChatServer::handleNewConnection()
{
    const std::string methodName = "ChatServer::handleNewConnection()"; 

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0)
    {
        Logger::errorWithErrno("Failed to accept new connection", methodName);
        return;
    }

    fcntl(client_fd, F_SETFL, O_NONBLOCK); // Set the client socket to non-blocking mode

    clients[client_fd] = ""; // Temporarily store an empty nickname
    FD_SET(client_fd, &master_set);
    if (client_fd > max_fd)
    {
        max_fd = client_fd; // Update the maximum file descriptor
    }

    std::stringstream ss;
    ss << "New client connected: " << std::to_string(client_fd);

    Logger::info(ss.str(), methodName);
    askForNickname(client_fd); // Ask the new client for a nickname
}

void ChatServer::handleClientMessage(int client_fd)
{
    const std::string methodName = "ChatServer::handleClientMessage";

    char buffer[1024];
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0)
    {
        // Handle client disconnection
        close(client_fd);
        FD_CLR(client_fd, &master_set);

        std::string nickname = clients[client_fd];
        clients.erase(client_fd);

        if (!nickname.empty())
        {
            std::string leave_message = nickname + " has left the chat.";
            broadcastMessage(leave_message);
        }

        std::stringstream ss;
        ss << "Client disconnected: " << std::to_string(client_fd);
        Logger::info(ss.str(), methodName);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string message(buffer);

    if (clients[client_fd].empty())
    {
        // If the client has not set a nickname, treat the first message as their nickname
        setNickname(client_fd, message);
    }
    else
    {
        // Broadcast the message with the client's nickname
        std::string full_message = clients[client_fd] + ": " + message;
        broadcastMessage(full_message, client_fd);
    }
}

void ChatServer::broadcastMessage(const std::string& message, int exclude_fd)
{
    const std::string methodName = "ChatServer::broadcastMessage()";
    for (const auto& [client_fd, nickname] : clients)
    {
        if (client_fd != exclude_fd)
        {
            send(client_fd, message.c_str(), message.size(), 0); // Send message to clients
        }
    }

    std::stringstream ss;
    ss << "Broadcast message: " << message;

    Logger::info(ss.str(), methodName);
}

ChatServer::~ChatServer()
{
    close(server_fd); // Close the server socket
    for (const auto& [client_fd, nickname] : clients)
    {
        close(client_fd); // Close each client socket
    }
}
