#ifndef LAUNCH_LIB_H
#define LAUNCH_LIB_H

#ifdef _WIN32
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

int launch_program(const char* command, bool wait_for_exit);

#endif