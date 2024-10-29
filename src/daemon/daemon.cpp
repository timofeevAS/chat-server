// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#include "../pch.h"


void Daemon::handleSignal(int signal)
{
    const std::string methodName = "Daemon::handleSignal()";

    switch (signal)
    {
    case SIGTERM:
        Logger::info("Caught SIGTERM.", methodName);
        exit(EXIT_SUCCESS);
        break;
    default:
        std::stringstream ss;
        ss << "Caught unexpected signal: " << strsignal(signal);
        
        Logger::info(ss.str(), methodName);
        break;
    }

    Logger::info("Finished!", methodName);
}

void Daemon::start()
{
    const std::string methodName = "Daemon::start()";

    pid_t pid = fork();

    if (pid < 0)
    {
        Logger::errorWithErrno("Fork process failed.", methodName);
        exit(EXIT_FAILURE);
    }

    if (pid > 0)
    {
        std::stringstream ss;
        ss << "Parent process is terminating, PID: " << getpid();
        
        Logger::info(ss.str(), methodName);
        exit(EXIT_SUCCESS);
    }

    pid_t sid = setsid();

    if (sid < 0)
    {
        Logger::errorWithErrno("SID setting was failed", methodName);
        exit(EXIT_FAILURE);
    }

    umask(0);

    if (chdir(WORKING_DIR.c_str()) < 0)
    {
        Logger::errorWithErrno("Changing working directory was failed", methodName);
        exit(EXIT_FAILURE);
    }

    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);

    signal(SIGTERM, handleSignal);
    Logger::info("Finished!", methodName);
}