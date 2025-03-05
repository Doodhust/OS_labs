/*
  Simulate temperature measuring device. Each minute it calls the OpenWeatherMap
  API to get actual temperature. That's where cURL and Boost::json is needed.
  COM-port is simulated with named pipes because their interface is similar.
*/

#include <chrono>
#include <ctime>
#include <random>
#include <thread>

#include "utils/measur.h"
#include "utils/npipe/npipe.h"


// write measurements to named pipe infinitely
int main(int argc, char *argv[]) {
  using namespace std::chrono_literals;

  NPipe<Measur> npipe("random_temp_pipe");

  std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());
  std::normal_distribution<float> norm_distr(0, 0.1);

  Measur measur = {.real_temp = -5.0, .felt_temp = -10.0, .timestamp = time(0)};
  while (true) {
    measur.real_temp += norm_distr(rng);
    measur.felt_temp += norm_distr(rng);
    measur.timestamp = time(0);
    npipe.Write(&measur);
    std::this_thread::sleep_for(5s);
  }

  return 0;
}
