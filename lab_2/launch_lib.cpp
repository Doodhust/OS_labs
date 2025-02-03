#include "launch_lib.h"
#include <cstring>
#include <cstdlib>
#include <iostream>

int launch_program(const char* command, bool wait_for_exit) {
#ifdef _WIN32
    // Windows implementation
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));

    // Convert command to non-const string
    char* cmd = _strdup(command);
    if (!cmd) {
        std::cerr << "Memory allocation failed!" << std::endl;
        return -1;
    }

    if (!CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess failed: " << GetLastError() << std::endl;
        free(cmd);
        return -1;
    }

    free(cmd);

    if (wait_for_exit) {
        WaitForSingleObject(pi.hProcess, INFINITE);
        DWORD exit_code;
        GetExitCodeProcess(pi.hProcess, &exit_code);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        return exit_code;
    }

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    return 0;

#else
    // Unix implementation
    pid_t child_pid = fork();

    if (child_pid == -1) {
        std::cerr << "Fork failed!" << std::endl;
        return -1;
    } else if (child_pid == 0) {
        // Child process
        execl("/bin/sh", "sh", "-c", command, NULL);
        std::cerr << "Exec failed!" << std::endl;
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        if (wait_for_exit) {
            int status;
            waitpid(child_pid, &status, 0);
            if (WIFEXITED(status)) {
                return WEXITSTATUS(status);
            }
            return -1;
        }
        return 0;
    }
#endif
}