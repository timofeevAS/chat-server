// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "../pch.h"

std::string trim(const std::string & str)
{
    size_t first = str.find_first_not_of(" \n\r\t");
    if (first == std::string::npos) return ""; // Empty string case

    size_t last = str.find_last_not_of(" \n\r\t");
    return str.substr(first, (last - first + 1));
}

ChatServer::ChatServer(int port)
{
    const std::string methodName = "ChatServer::ChatServer";
    Logger::info("Start", methodName);

    setupServerSocket(port); // Initialize the server socket
    FD_ZERO(&master_set);    // Initialize the master file descriptor set
    FD_SET(server_fd, &master_set); // Add the server_fd to the master set
    max_fd = server_fd;      // Set the initial maximum file descriptor

    Logger::info("Finished", methodName);
}

void ChatServer::setupServerSocket(int port)
{
    const std::string methodName = "ChatServer::setupServerSocket";
    Logger::info("Start", methodName);

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
    Logger::info("Finished", methodName);
}

void ChatServer::run()
{
    const std::string methodName = "ChatServer::run";
    Logger::info("Start", methodName);

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

    Logger::info("Finished", methodName);
}

void ChatServer::askForNickname(int client_fd)
{
    const std::string methodName = "ChatServer::askForNickname";
    Logger::info("Start", methodName);

    std::string message = "Please enter your nickname: ";
    send(client_fd, message.c_str(), message.size(), 0); // Request nickname from the new client

    Logger::info("Finished", methodName);
}

void ChatServer::setNickname(int client_fd, const std::string& nickname)
{
    const std::string methodName = "ChatServer::setNickname";
    Logger::info("Start", methodName);

    // Check if the nickname is already taken
    for (const auto& [fd, existing_nickname] : clients) {
        if (existing_nickname == nickname) {
            // Notify client that the nickname is taken
            std::string error_msg = "Nickname already taken. Connection will be closed.\n";
            send(client_fd, error_msg.c_str(), error_msg.size(), 0);
            Logger::info("Nickname taken, disconnecting client", methodName);

            // Disconnect client
            close(client_fd);
            FD_CLR(client_fd, &master_set);
            clients.erase(client_fd);
            return;
        }
    }

    // Assign the unique nickname to the client
    clients[client_fd] = nickname;

    std::stringstream ss;
    ss << "Client " << std::to_string(client_fd) << " set nickname to " << nickname;
    Logger::info(ss.str(), methodName);

    // Notify other users that a new client has joined
    std::string join_message = nickname + " has joined the chat!";
    broadcastMessage(join_message, client_fd);

    Logger::info("Finished", methodName);
}

void ChatServer::handleNewConnection()
{
    const std::string methodName = "ChatServer::handleNewConnection";
    Logger::info("Start", methodName);

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

    Logger::info("Finished", methodName);
}

void ChatServer::handleClientMessage(int client_fd)
{
    const std::string methodName = "ChatServer::handleClientMessage";
    Logger::info("Start", methodName);

    char buffer[1024];
    int bytes_read = recv(client_fd, buffer, sizeof(buffer), 0);

    if (bytes_read <= 0)
    {
        // Log entry before attempting disconnection
        Logger::info("Preparing to disconnect client " + std::to_string(client_fd), methodName);

        // Handle client disconnection
        close(client_fd);
        FD_CLR(client_fd, &master_set);

        auto client_it = clients.find(client_fd);
        std::string nickname;
        if (client_it != clients.end()) {
            nickname = client_it->second;
            clients.erase(client_it); // Safely erase client from the map
        }

        if (!nickname.empty())
        {
            std::string leave_message = nickname + " has left the chat.";
            broadcastMessage(leave_message, client_fd); // Inform others the client left
        }

        std::stringstream ss;
        ss << "Client disconnected: " << client_fd;
        Logger::info(ss.str(), methodName);

        Logger::info("Finished", methodName);
        return;
    }

    buffer[bytes_read] = '\0';
    std::string message = trim(buffer);

    // Skip processing if message is empty
    if (message.empty()) {
        Logger::info("Message empty.", methodName);
        Logger::info("Finished", methodName);
        return;
    }

    // Check if the client has set a nickname; if not, treat the message as nickname input
    if (clients[client_fd].empty())
    {
        setNickname(client_fd, message);

        // Send a welcome message to the user
        std::string welcome_msg = "Welcome to the chat, " + clients[client_fd] + "!\n";
        welcome_msg += "Type /? to see available commands.\n";
        if (send(client_fd, welcome_msg.c_str(), welcome_msg.size(), 0) < 0) {
            Logger::errorWithErrno("Failed to send welcome message", methodName);
        }

        std::string user_list = "\nActive users:\n";
        for (const auto& [fd, nickname] : clients)
        {
            if (!nickname.empty()) // Only include users with set nicknames
                user_list += " - " + nickname + "\n";
        }

        if (send(client_fd, user_list.c_str(), user_list.size(), 0) < 0) {
            Logger::errorWithErrno("Failed to send user list", methodName);
        }
    }
    else if (!message.empty() && message[0] == '/')
    {
        // Process the message as a command if it starts with '/'
        processCommand(client_fd, message);
    }
    else
    {
        // Otherwise, treat the message as a broadcast
        std::stringstream ss;
        ss << clients[client_fd] << ": " << message << "\n";
        broadcastMessage(ss.str(), client_fd);
    }

    Logger::info("Finished", methodName);
}


void ChatServer::broadcastMessage(const std::string& message, int exclude_fd)
{
    const std::string methodName = "ChatServer::broadcastMessage";
    Logger::info("Start", methodName);

    std::string output = message + "\n";

    for (const auto& [client_fd, nickname] : clients)
    {
        // Skip clients without a nickname and the excluded client
        if (client_fd != exclude_fd && !nickname.empty())
        {
            send(client_fd, output.c_str(), output.size(), 0);
        }
    }

    std::stringstream ss;
    ss << "Broadcast message: " << message;
    Logger::info(ss.str(), methodName);

    Logger::info("Finished", methodName);
}


void ChatServer::processCommand(int client_fd, const std::string& command)
{
    const std::string methodName = "ChatServer::processCommand";
    Logger::info("Start", methodName);

    std::istringstream iss(command);
    std::string cmd, arg1, arg2;

    iss >> cmd; // Get command

    if (cmd == "/send" || cmd == "/s")
    {
        iss >> arg1; // Receiver name
        std::getline(iss, arg2); // Message

        arg2 = trim(arg2); // Remove spaces and line feeds
        if (arg1.empty() || arg2.empty())
        {
            send(client_fd, "Usage: /send <name> <message>\n", 29, 0);
            Logger::info("Finished", methodName);
            return;
        }

        // Find recipient by nickname and check if it has a nickname
        int recipient_fd = -1;
        for (const auto& [fd, nickname] : clients) {
            if (nickname == arg1 && !nickname.empty()) {
                recipient_fd = fd;
                break;
            }
        }

        if (recipient_fd != -1 && !clients[client_fd].empty())
        {
            std::string private_msg = "[!] Message from " + clients[client_fd] + ": " + arg2 + "\n";
            send(recipient_fd, private_msg.c_str(), private_msg.size(), 0);
        }
        else if (clients[client_fd].empty())
        {
            send(client_fd, "Please set a nickname before sending messages.\n", 46, 0);
        }
        else
        {
            send(client_fd, "User not found\n", 15, 0);
        }
    }
    else if (cmd == "/all" || cmd == "/a")
    {
        std::getline(iss, arg1);
        arg1 = trim(arg1);

        if (arg1.empty())
        {
            send(client_fd, "Usage: /all <message>\n", 22, 0);
            Logger::info("Finished", methodName);
            return;
        }

        std::string broadcast_msg = "[All] " + clients[client_fd] + ": " + arg1;
        broadcastMessage(broadcast_msg, client_fd);
    }
    else if (cmd == "/users" || cmd == "/u")
    {
        std::string user_list = "Active users:\n";
        for (const auto& [fd, nickname] : clients)
        {
            if (!nickname.empty())
            {
                // Only include users with set nicknames
                user_list += " - " + nickname + "\n";
            }
        }
        send(client_fd, user_list.c_str(), user_list.size(), 0);
    }
    else if (cmd == "/quit" || cmd == "/q")
    {
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
        ss << "Client " << nickname << " disconnected with /quit";
        Logger::info(ss.str(), methodName);
    }
    else if (cmd == "/?" || cmd == "/help")
    {
        std::string help_message = "Available commands:\n";
        help_message += "/send <name> <message> or /s <name> <message> - Send a private message to a user\n";
        help_message += "/all <message> or /a <message> - Send a broadcast message to all users\n";
        help_message += "/users or /u - List all active users\n";
        help_message += "/quit or /q - Quit the chat\n";
        help_message += "/? or /help - Show this help message\n";
        
        send(client_fd, help_message.c_str(), help_message.size(), 0);
    }
    else
    {
        send(client_fd, "Unknown command: use /help\n", 16, 0);
    }

    Logger::info("Finished", methodName);
}

ChatServer::~ChatServer()
{
    const std::string methodName = "ChatServer::~ChatServer";
    Logger::info("Start", methodName);

    close(server_fd); // Close the server socket
    for (const auto& [client_fd, nickname] : clients)
    {
        close(client_fd); // Close each client socket
    }

    Logger::info("Finished", methodName);
}
