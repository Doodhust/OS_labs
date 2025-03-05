/*
  Named pipe: create, open, write measurements, read, close, unlink.
  NPipe class encapsulates named pipe name and its representation, different for
  different OSs.
*/

#pragma once

#include <memory>
#include <string>


// encapsulate named pipe name and representation, different for different OS
template <class T>
class NPipe {
 public:
  // init named pipe with the given name and its handler
  explicit NPipe(const char *name, bool winserver = true);

  // cleanup and free resources
  ~NPipe();

  // write T to named pipe
  long long Write(const T *data);

  // blocking read T from named pipe
  T Read();

 private:
  // "/tmp/{name}" on Linux, "\\.\pipe\{name}" on Windows
  std::string name_;

  // fd on Linux, HANDLER on Windows
  struct Handler;
  std::unique_ptr<Handler> handler_;
};
