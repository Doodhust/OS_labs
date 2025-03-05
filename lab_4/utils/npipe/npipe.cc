/*
  Named pipe: create, open, write measurements, read, close, unlink.
  This implementation is for POSIX systems.
*/

#include "npipe.h"

#include "utils/measur.h"


#ifndef _WIN32

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>


// encapsulates named pipe representation
// on linux it just file descriptor
template <class T>
struct NPipe<T>::Handler {
  Handler() : fd(-1) {
  }

  int fd;
};


// init named pipe with the given name and its handler
// on linux name is "/tmp/{name}"
template <class T>
NPipe<T>::NPipe(const char *name, bool _)
: name_(std::string("/tmp/") + name)
, handler_(std::make_unique<NPipe<T>::Handler>()) {
  struct stat st;
  // get file status
  if (stat(name_.c_str(), &st) != 0) {
    perror("stat");
  }
  // if named pipe doesn't exist, create it
  if (!(S_ISFIFO(st.st_mode))) {
    if (mkfifo(name_.c_str(), 0666) == -1) {
      perror("mkfifo");
      exit(EXIT_FAILURE);
    }
  }
  // open named pipe for read and write
  handler_->fd = open(name_.c_str(), O_RDWR);
  if (handler_->fd == -1) {
    perror("open");
  }
}

// close fd and unlink named pipe
template <class T>
NPipe<T>::~NPipe() {
  if (handler_->fd != -1) {
    close(handler_->fd);
    unlink(name_.c_str());
  }
}

// write T to named pipe
template <class T>
long long NPipe<T>::Write(const T *data) {
  return write(handler_->fd, data, sizeof(T));
}

// blocking read T from named pipe
template <class T>
T NPipe<T>::Read() {
  T data;
  ssize_t bytes_read = read(handler_->fd, &data, sizeof(T));
  if (bytes_read == -1) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  return data;
}

#else

#include <windows.h>

#include <iostream>


template <class T>
NPipe<T>::NPipe(const char* name, bool winserver)
: name_(std::string("\\\\.\\pipe\\") + name)
, handler_(std::make_unique<NPipe<T>::Handler>()) {
  // when server calls constructor
  if (winserver) {
    // create new pipe
    handler_->pipe_handle =
      CreateNamedPipeA(name_.c_str(),
                       PIPE_ACCESS_DUPLEX,
                       PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                       1,
                       sizeof(T),
                       sizeof(T),
                       0,
                       nullptr);

    if (handler_->pipe_handle == INVALID_HANDLE_VALUE) {
      std::cerr << "Failed to create pipe: " << GetLastError() << std::endl;
      exit(EXIT_FAILURE);
    }

    // wait for a client to connect
    if (!ConnectNamedPipe(handler_->pipe_handle, nullptr)) {
      std::cerr << "Failed to connect to client: " << GetLastError()
                << std::endl;
      CloseHandle(handler_->pipe_handle);
      exit(EXIT_FAILURE);
    }
  } else {
    // when the client calls constructor
    // just open existing pipe
    handler_->pipe_handle = CreateFileA(name_.c_str(),
                                        GENERIC_READ | GENERIC_WRITE,
                                        0,
                                        nullptr,
                                        OPEN_EXISTING,
                                        0,
                                        nullptr);

    if (handler_->pipe_handle == INVALID_HANDLE_VALUE) {
      std::cerr << "Failed to open existing pipe: " << GetLastError()
                << std::endl;
      exit(EXIT_FAILURE);
    }
  }
}

template <class T>
NPipe<T>::~NPipe() {
  if (handler_->pipe_handle != INVALID_HANDLE_VALUE) {
    DisconnectNamedPipe(handler_->pipe_handle);
    CloseHandle(handler_->pipe_handle);
  }
}

template <class T>
long long NPipe<T>::Write(const T* data) {
  DWORD bytes_written;
  BOOL success = WriteFile(handler_->pipe_handle,
                           data,
                           sizeof(T),
                           &bytes_written,
                           nullptr  // No overlapped structure
  );

  if (!success) {
    std::cerr << "WriteFile failed. Error: " << GetLastError() << std::endl;
    return -1;
  }
  return static_cast<long long>(bytes_written);
}

template <class T>
T NPipe<T>::Read() {
  T data;
  DWORD bytes_read;
  BOOL success = ReadFile(handler_->pipe_handle,
                          &data,
                          sizeof(T),
                          &bytes_read,
                          nullptr  // No overlapped structure
  );

  if (!success || bytes_read != sizeof(T)) {
    std::cerr << "ReadFile failed. Error: " << GetLastError() << std::endl;
    exit(EXIT_FAILURE);
  }

  return data;
}

#endif


// instantiate specialization for Measur
template class NPipe<Measur>;
