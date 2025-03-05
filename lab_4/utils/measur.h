/*
  Struct to be parsed from OpenWeatherMap API response, written into named pipe
  and read from it.
*/

#pragma once

#include <ctime>
#include <string>

struct Measur {
  float real_temp, felt_temp;
  time_t timestamp;
};
