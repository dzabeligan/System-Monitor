#include "linux_parser.h"

#include <dirent.h>
#include <unistd.h>

#include <sstream>
#include <string>
#include <vector>
#include <numeric>

using std::getline;
using std::ifstream;
using std::istringstream;
using std::replace;
using std::stof;
using std::stol;
using std::string;
using std::to_string;
using std::vector;

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
        if (key == "PRETTY_NAME") {
          replace(value.begin(), value.end(), '_', ' ');
          return value;
        }
      }
    }
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
  float memTotal = 0.0;
  float memFree = 0.0;
  ifstream filestream(kProcDirectory + kMeminfoFilename);

  if (filestream.is_open()) {
    string line;
    int writtenCount = 0;

    while (getline(filestream, line)) {
      string value;
      string key;
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == "MemTotal:") {
          memTotal = stof(value);
          writtenCount++;
        }
        if (key == "MemFree:") {
          memFree = stof(value);
          writtenCount++;
        }
      }

      if (writtenCount == 2) {
        break;
      }
    }
  }
  return (memTotal - memFree) / memTotal;
}

/**
 * @brief Reads and returns the system uptime
 *
 * @return long
 */
long LinuxParser::UpTime() {
  string uptime;
  long lUptime;
  ifstream stream(kProcDirectory + kUptimeFilename);

  if (stream.is_open()) {
    string line;
    getline(stream, line);
    istringstream linestream(line);
    linestream >> uptime;
  }
  try {
    lUptime = stol(uptime);
  } catch (...) {
    lUptime = 0;
  }
  return lUptime;
}

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
      return user + kernel + children_user + children_kernel;
    }
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

      if (key == "cpu") {
        long value;
        while (linestream >> value) {
          cpu.emplace_back(value);
        }

        return cpu;
      }
    }
  }
  return cpu;
}

/**
 * @brief Reads and returns an integer value for a given key
 *
 * @param rKey
 * @return int
 */
static int readKeyFromStatFile_Int(const string& rKey) {
  ifstream filestream(LinuxParser::kProcDirectory + LinuxParser::kStatFilename);

  if (filestream.is_open()) {
    string line;

    while (getline(filestream, line)) {
      string key;
      string value;
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == rKey) {
          return std::stoi(value);
        }
      }
    }
  }
  return 0;
}

/**
 * @brief Reads and returns the total number of processes
 *
 * @return int
 */
int LinuxParser::TotalProcesses() {
  return readKeyFromStatFile_Int("processes");
}

/**
 * @brief Reads and returns the number of running processes
 *
 * @return int
 */
int LinuxParser::RunningProcesses() {
  return readKeyFromStatFile_Int("procs_running");
}

/**
 * @brief Reads and returns the command associated with a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Command(int pid) {
  string value;
  ifstream filestream(kProcDirectory + to_string(pid) + kCmdlineFilename);

  if (filestream.is_open()) {
    string line;
    getline(filestream, line);
    replace(line.begin(), line.end(), ':', ' ');
    istringstream linestream(line);
    linestream >> value;
  }
  return value;
}

/**
 * @brief Reads and returns the memory used by a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Ram(int pid) {
  string value;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    string line;

    while (getline(filestream, line)) {
      string key;
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == "VmRSS:") {
          return value;
        }
      }
    }
  }
  return value;
}

/**
 * @brief Reads and returns the user ID associated with a process
 *
 * @param pid
 * @return string
 */
string LinuxParser::Uid(int pid) {
  string value;
  ifstream filestream(kProcDirectory + to_string(pid) + kStatusFilename);

  if (filestream.is_open()) {
    string line;

    while (getline(filestream, line)) {
      string key;
      istringstream linestream(line);

      while (linestream >> key >> value) {
        if (key == "Uid:") {
          return value;
        }
      }
    }
  }
  return value;
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
        return user;
      }
    }
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
    return uptime;
  }
  return 0;
}
