// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "pch.h"
#include "daemon/daemon.h"
#include "logger/logger.h"

int main(int argc, char const *argv[])
{
    // Starting logger.
    Logger::init(argv[0]);

    // Starting daemon.
    Daemon::start();

    return 0;
}