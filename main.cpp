// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "pch.h"
#include "daemon/daemon.h"

std::string APP_NAME;

int main(int argc, char const *argv[])
{
    APP_NAME = argv[0];

    // Open log
    openlog(APP_NAME.c_str(), LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_INFO, "Started chat server: %s", APP_NAME.c_str());

    // Starting daemon
    Daemon::start();

    return 0;
}