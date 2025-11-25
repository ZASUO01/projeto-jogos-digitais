//
// Created by pedro-souza on 24/11/2025.
//
#include "Logger.h"
#include <cstdio>
#include <cerrno>
#include <cstdlib>
#include <cstring>

void Logger::sysLogExit(const char *msg) {
    const int err = errno;
    fprintf(stderr, "[SYSCALL ERROR] %s: %s\n", msg, strerror(err));
    exit(EXIT_FAILURE);
}