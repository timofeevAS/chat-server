// Copyright: Alexandr Timofeev 2024 - telegram: timofeev2624.
#ifndef PCH_H
#define PCH_H

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <map>

#include <getopt.h>
#include <syslog.h>
#include <signal.h>
#include <dirent.h>
#include <sched.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <sys/socket.h>

#include "../include/logger/logger.h"
#include "../include/server/server.h"
#include "../include/daemon/daemon.h"

static inline const std::string WORKING_DIR = "/tmp/";


#endif