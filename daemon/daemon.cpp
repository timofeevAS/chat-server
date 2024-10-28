// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "pch.h"
#include "daemon.h"

void Daemon::handleSignal(int signal)
{
    switch (signal)
    {
    case SIGTERM:
        syslog(LOG_INFO, "Daemon::handleSignal(): Caught SIGTERM.");
        exit(EXIT_SUCCESS);
        break;
    default:
        syslog(LOG_ERR, "Daemon::handleSignal(): Caught unexpected signal: %d", strsignal(signal));
        break;
    }
    syslog(LOG_INFO, "Daemon::handleSignal(): Finished!");
}

void Daemon::start()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        syslog(LOG_ERR, "Daemon::start(): Fork process failed with next error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        syslog(LOG_INFO, "Daemon::start(): Parent process is terminating, PID: %d", getpid());
        exit(EXIT_SUCCESS);
    }

    pid_t sid = setsid();

    if (sid < 0)
    {
        syslog(LOG_ERR, "Daemon::start(): SID setting is failed with error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    umask(0);

    if (chdir(WORKING_DIR.c_str()) < 0)
    {
        syslog(LOG_ERR, "Daemon::start(): Changing working directory was failed with error: %s", strerror(errno));
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, handleSignal);
    syslog(LOG_INFO, "Daemon::start(): Finished!");
}