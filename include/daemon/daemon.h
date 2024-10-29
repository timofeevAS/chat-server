// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#ifndef DAEMON_H
#define DAEMON_H

class Daemon
{
private:
    static void handleSignal(int signal);

public:
    static void start();
};


#endif