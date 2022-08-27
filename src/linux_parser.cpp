#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <algorithm>
#include <iomanip>
#include <numeric>
#include <sstream>
#include <string>
#include <vector>

using std::getline;
using std::ifstream;
using std::istringstream;
using std::replace;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

template <typename T>
T findValueByKey(string const& keyFilter, string const& filename) {
  T value;

  ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    string line, key;
    while (getline(stream, line)) {
      istringstream linestream(line);
      while (linestream >> key >> value) {
        if (key == keyFilter) {
          stream.close();
          return value;
        }
      }
    }
    stream.close();
  }
  return value;
};

template <typename T>
T getValueOfFile(string const& filename) {
  T value;

  ifstream stream(LinuxParser::kProcDirectory + filename);
  if (stream.is_open()) {
    string line;
    getline(stream, line);
    istringstream linestream(line);
    linestream >> value;
    stream.close();
  }
  return value;
};
/**
 * @brief Reads and returns the operating system name
 *
 * @return string
 */
string LinuxParser::OperatingSystem() {
  string value;
  ifstream filestream(kOSPath);

  if (filestream.is_open()) {
    string line;

    while (getline(filestream, line)) {
      replace(line.begin(), line.end(), ' ', '_');
      replace(line.begin(), line.end(), '=', ' ');
      replace(line.begin(), line.end(), '"', ' ');

      string key;
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == filterOS) {
          replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
    filestream.close();
  }
  return value;
}

/**
 * @brief Reads and returns the system kernel name
 *
 * @return string
 */
string LinuxParser::Kernel() {
  string kernel;
  ifstream stream(kProcDirectory + kVersionFilename);

  if (stream.is_open()) {
    string line;
    string os, version;
    getline(stream, line);
    istringstream linestream(line);
    linestream >> os >> version >> kernel;
    stream.close();
  }
  return kernel;
}

// BONUS: Update this to use std::filesystem
vector<int> LinuxParser::Pids() {
  vector<int> pids;
  DIR* directory = opendir(kProcDirectory.c_str());
  struct dirent* file;

  while ((file = readdir(directory)) != nullptr) {
    // Is this a directory?
    if (file->d_type == DT_DIR) {
      // Is every character of the name a digit?
      string filename(file->d_name);
      if (std::all_of(filename.begin(), filename.end(), isdigit)) {
        int pid = stoi(filename);
        pids.emplace_back(pid);
      }
    }
  }
  closedir(directory);
  return pids;
}

/**
 * @brief Reads and returns the system memory utilization
 *
 * @return float
 */
float LinuxParser::MemoryUtilization() {
  // "/proc/memInfo"
  float Total = findValueByKey<float>(filterMemTotalString, kMeminfoFilename);
  float Free = findValueByKey<float>(filterMemFreeString, kMeminfoFilename);
  return (Total - Free) / Total;
}

/**
 * @brief Reads and returns the system uptime
 *
 * @return long
 */
long LinuxParser::UpTime() { return getValueOfFile<long>(kUptimeFilename); }

/**
 * @brief Returns the number of jiffies for the system
 *
 * @return long
 */
long LinuxParser::Jiffies() { return UpTime() * sysconf(_SC_CLK_TCK); }

/**
 * @brief Returns the number of active jiffies for a PID
 *
 * @param pid
 * @return long
 */
long LinuxParser::ActiveJiffies(int pid) {
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    int jiffiesOffsetSub1 = 12;
    long user, kernel, children_user, children_kernel;
    string line;
    string value;
    getline(filestream, line);
    istringstream linestream(line);

    while (linestream >> value && jiffiesOffsetSub1-- != 0)
      ;

    while (linestream >> user >> kernel >> children_user >> children_kernel) {
      filestream.close();
      return user + kernel + children_user + children_kernel;
    }
    filestream.close();
  }
  return 0;
}

/**
 * @brief Returns the number of active jiffies for the system
 *
 * @return long
 */
long LinuxParser::ActiveJiffies() {
  std::vector<long> cpu(CpuUtilization());
  return std::accumulate(cpu.begin(), cpu.end(), 0L) - IdleJiffies();
}

/**
 * @brief Returns the number of idle jiffies for the system
 *
 * @return long
 */
long LinuxParser::IdleJiffies() {
  std::vector<long> cpu(CpuUtilization());
  return cpu[CPUStates::kIdle_] + cpu[CPUStates::kIOwait_];
}

/**
 * @brief Reads and returns CPU utilization
 *
 * @return vector<long>
 */
vector<long> LinuxParser::CpuUtilization() {
  vector<long> cpu;
  ifstream filestream(kProcDirectory + kStatFilename);

  if (filestream.is_open()) {
    string line;
    while (getline(filestream, line)) {
      string key;
      istringstream linestream(line);
      linestream >> key;

      if (key == filterCpu) {
        long value;
        while (linestream >> value) {
          cpu.emplace_back(value);
        }

        filestream.close();
        return cpu;
      }
    }
    filestream.close();
  }
  return cpu;
}

/**
 * @brief Reads and returns the total number of processes
 *
 * @return int
 */
int LinuxParser::TotalProcesses() {
  return findValueByKey<int>(filterProcesses, kStatFilename);
}

/**
 * @brief Reads and returns the number of running processes
 *
 * @return int
 */
int LinuxParser::RunningProcesses() {
  return findValueByKey<int>(filterRunningProcesses, kStatFilename);
}

/**
 * @brief Reads and returns the command associated with a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Command(int pid) {
  return string(getValueOfFile<string>(to_string(pid) + kCmdlineFilename));
}

/**
 * @brief Reads and returns the memory used by a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Ram(int pid) {
  std::stringstream stream;
  float Ram =
      findValueByKey<float>(filterProcMem, to_string(pid) + kStatusFilename);
  // convert to MB
  stream << std::fixed << std::setprecision(2) << (Ram / 1024);
  return stream.str();
}

/**
 * @brief Reads and returns the user ID associated with a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Uid(int pid) {
  return findValueByKey<string>(filterUID, to_string(pid) + kStatusFilename);
}

/**
 * @brief Reads and returns the user associated with a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::User(int pid) {
  const string userId = Uid(pid);
  string user;
  ifstream filestream(kPasswordPath);

  if (filestream.is_open()) {
    string line;

    while (getline(filestream, line)) {
      string x;
      string id;
      replace(line.begin(), line.end(), ':', ' ');
      istringstream linestream(line);
      while (linestream >> user >> x >> id && id == userId) {
        filestream.close();
        return user;
      }
    }
    filestream.close();
  }
  return user;
}

/**
 * @brief Reads and returns the uptime of a process
 *
 * @param pid
 * @return long
 */
long LinuxParser::UpTime(int pid) {
  ifstream filestream(kProcDirectory + to_string(pid) + kStatFilename);

  if (filestream.is_open()) {
    int uptimeOffset = 21;
    long uptime;
    string line;
    string value;
    getline(filestream, line);
    istringstream linestream(line);

    while (linestream >> value && uptimeOffset-- != 0)
      ;

    try {
      uptime = stol(value);
    } catch (...) {
      uptime = 0;
    }
    filestream.close();
    if (Kernel() > "2.6") {
      uptime = UpTime() - uptime / sysconf(_SC_CLK_TCK);
    }

    return uptime;
  }
  return 0;
}
