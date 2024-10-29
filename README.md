# About
In this repository represent attempt of solution for study task.

* Discipline: UNIX & Linux OS;
* Theme: chat-server with non-blocking sockets;

## Task

The server should use non-blocking sockets to handle parallel processing and data transmission to clients. The network server must be implemented as a daemon, with error handling and logging either through `syslog` or to a log file. The server should support both unicast and broadcast messaging, allowing text messages to be sent to a single participant or all participants, respectively. A network client should be implemented to test the server's functionality. The network client can be implemented without non-blocking sockets; for a simple version, utilities like `netcat` or `telnet` can be used.

## Description

`TBD`

## How to run

```
make
./bin/chat-server
nc localhost 8008
```