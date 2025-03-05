/*
  Reads measurments from named pipe. Reading is blocking. As soon as new
  measurements arrive they are read with this program and settled in log files.

  There are three log files, each keeping records not older than specific time
  cutoff.

  Three threads are doing their job:

  1) the first thread reads the measurements from the named pipe into which they
  were written by the device and appends new record to first log file. Expired
  records in first log, i.e. those that are older than CUTOFF[1], are erased.

  2) the second thread counts average on those first log file records that are
  not older than CUTOFF[0]. The average is written into second log file. Expired
  record in second log, i.e. those that are older than CUTOFF[2], are erased.
  Then the thread sleeps for CUTOFF[0].

  3) the third thread counts average on those second log file records that are
  not older than CUTOFF[1]. The average is written into third log file. Expired
  record in third log, i.e. those that are older than CUTOFF[3], are erased.
  Then the thread sleeps for CUTOFF[1].

  Records in log files are done in the following format:
  <verbose timestamp> (<timestamp>) - Temperature <value>°C feels like <value>°C
  Thu Jan 16 14:17:41 2025 (1737001061) - Temperature -2.69°C feels like -8.93°C
*/

#include <cstring>
#include <ctime>
#include <format>
#include <fstream>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "utils/measur.h"
#include "utils/npipe/npipe.h"


// how many seconds are contained in a given unit
const int MINUTE = 60;
const int HOUR = 60 * MINUTE;
const int DAY = 24 * HOUR;
const int WEEK = 7 * DAY;
const int MONTH = 30 * DAY;
const int YEAR = 365 * DAY;

// Time cutoffs controlling when to count average and erase old records
// (docstrings at the beginning makes it clear)
int CUTOFF[] = {HOUR, DAY, MONTH, YEAR};
// note: reduce cutoffs to make changes visible and to debug it
// int CUTOFF[] = {30, 120, 300, 600};  // uncomment this

// log file names and mutex's
const std::string LOG_NAME[] = {"first.log", "second.log", "third.log"};
std::mutex log_mutex[3];


// Format measurement into verbose string that will be written into log file
// <verbose timestamp> (<timestamp>) - Temperature <val>°C feels like <val>°C
std::string MeasurToVerbose(Measur measur) {
  char *verb_timestamp = ctime((long *)&measur.timestamp);
  verb_timestamp[24] = ' ';
  return std::format("{}({}) - Temperature {:.2f}°C feels like {:.2f}°C",
                     verb_timestamp,
                     measur.timestamp,
                     measur.real_temp,
                     measur.felt_temp);
}


// Parse Measur from verbose, template:
// <verbose timestamp> (<timestamp>) - Temperature <val>°C feels like <val>°C
// There is only one opening and close parentheses, the timestamp is placed
// within them. The real temperature is placed between "Temperature " and "°C",
// and the temperature how it's felt is placed between "like " and "°C"
Measur MeasurFromVerbose(const std::string &verb) {
  auto timestamp = verb.substr(verb.find("(") + 1, verb.find(")"));
  auto real_temp = verb.substr(verb.find("Temperature ") + 12, verb.find("°C"));
  auto felt_temp = verb.substr(verb.find("like ") + 5, verb.rfind("°C"));
  Measur measur = {.real_temp = std::stof(real_temp),
                   .felt_temp = std::stof(felt_temp),
                   .timestamp = std::stoll(timestamp)};
  return measur;
}


// Computing average on records from log file with given index that are not
// older than CUTOFF[idx]
Measur ComputeAverage(int idx) {
  // acquire single access to log file
  std::lock_guard<std::mutex> lk(log_mutex[idx]);

  // compute average over records within the last CUTOFF[idx] second
  std::ifstream log_in(LOG_NAME[idx], std::ios::in);
  int cnt = 0;
  time_t now = time(0);
  Measur avg = {.real_temp = 0, .felt_temp = 0, .timestamp = now};

  std::string rec;
  while (std::getline(log_in, rec)) {
    Measur measur = MeasurFromVerbose(rec);
    // reading records that are not older that CUTOFF[idx] seconds
    if (now - measur.timestamp < CUTOFF[idx]) {
      avg.real_temp += measur.real_temp;
      avg.felt_temp += measur.felt_temp;
      cnt += 1;
    }
  }
  log_in.close();

  if (cnt > 0) {
    avg.real_temp /= cnt;
    avg.felt_temp /= cnt;
  }
  return avg;
}


// Read all records from log file with given index, and rewrite it back without
// too old records (older than CUTOFF[idx + 1]) and with new record
void RewriteLog(int idx, std::string new_record) {
  // capture mutex for reading/writing first.log
  std::lock_guard<std::mutex> lk(log_mutex[idx]);

  // reading log line by line
  std::ifstream log_in(LOG_NAME[idx], std::ios::in);
  std::vector<std::string> records;
  std::string rec;
  while (std::getline(log_in, rec)) {
    records.push_back(rec);
  }
  log_in.close();

  // pass the first expired records
  time_t now = time(0);
  int actual = 0;
  for (; actual < records.size(); ++actual) {
    if (now - MeasurFromVerbose(records[actual]).timestamp < CUTOFF[idx + 1]) {
      break;
    }
  }

  // rewrite log with actual records and a new one
  std::ofstream log_out(LOG_NAME[idx], std::ios::out | std::ios::trunc);
  for (int i = actual; i < records.size(); ++i) {
    log_out << records[i] << '\n';
  }
  log_out << new_record << '\n';
  log_out.close();
}


// Read the measurements from the named pipe into which they were written by
// the device and appends new record to first log file. Expired records in first
// log, i.e. those that are older than CUTOFF[1] seconds, are erased.
void WriteMeasurementsToFirst() {
  NPipe<Measur> npipe("random_temp_pipe");
  while (true) {
    // blocking reading
    Measur measur = npipe.Read();
    // write measurements as log record
    RewriteLog(0, MeasurToVerbose(measur));
  }
}


// Count average on those first log file records that are not older than
// CUTOFF[0]. The average is written into second log file. Expired record in
// second log, i.e. those that are older than CUTOFF[2] seconds, are erased.
// Sleep for CUTOFF[0] seconds between repeating this again and again.
void WriteAvgFromFirstToSecond() {
  // one can create chrono::duration with 10s, but not CUTOFF[0]s
  // so this is the workaround: explicitly call to operator""s
  auto sleep_dur = std::chrono_literals::operator""s(CUTOFF[0]);

  while (true) {
    std::this_thread::sleep_for(sleep_dur);
    Measur avg = ComputeAverage(0);
    RewriteLog(1, MeasurToVerbose(avg));
  }
}


// Count average on those second log file records that are not older than
// CUTOFF[1]. The average is written into third log file. Expired record in
// third log, i.e. those that are older than CUTOFF[3] seconds, are erased.
// Sleep for CUTOFF[1] seconds between repeating this again and again.
void WriteAvgFromSecondToThird() {
  // one can create chrono::duration with 10s, but not CUTOFF[0]s
  // so this is the workaround: explicitly call to operator""s
  auto sleep_dur = std::chrono_literals::operator""s(CUTOFF[1]);

  while (true) {
    std::this_thread::sleep_for(sleep_dur);
    Measur avg = ComputeAverage(1);
    RewriteLog(2, MeasurToVerbose(avg));
  }
}


int main(int argc, char *argv[]) {
  // when running with -d flag use less durations for erasing records and
  // computing averages
  if (argc > 1 and strcmp(argv[1], "-d") == 0) {
    const int DEBUG_CUTOFF[] = {30, 120, 300, 600};
    for (int i = 0; i < 4; ++i) {
      CUTOFF[i] = DEBUG_CUTOFF[i];
    }
  }

  // run until reading a device measurements runs
  std::thread write_to_second(WriteAvgFromFirstToSecond);
  std::thread write_to_third(WriteAvgFromSecondToThird);
  WriteMeasurementsToFirst();
  return 0;
}
