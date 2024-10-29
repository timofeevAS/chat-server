// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#ifndef SERVER_H
#define SERVER_H

class ChatServer
{
private:
    int server_fd;
    fd_set master_set;
    int max_fd;
    
    typedef std::map<int, std::string> ClientsMap;
    ClientsMap clients;  // Socket descriptor -> Nickname

    void setupServerSocket(int port);
    void handleNewConnection();
    void handleClientMessage(int client_fd);
    void broadcastMessage(const std::string& message, int exclude_fd = -1);

    void askForNickname(int client_fd);
    void setNickname(int client_fd, const std::string& nickname);

public:
    ChatServer(int port);
    ~ChatServer();
    void run();
};

#endif