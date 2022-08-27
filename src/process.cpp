#include "process.h"

#include <unistd.h>

#include <cctype>
#include <sstream>
#include <string>
#include <vector>

#include "linux_parser.h"

using std::string;
using std::to_string;
using std::vector;

/**
 * @brief Returns this process's ID
 *
 * @return int
 */
int Process::Pid() const { return pid_; }

/**
 * @brief Returns this process's CPU utilization
 *
 * @return float
 */
float Process::CpuUtilization() {
  return static_cast<float>(LinuxParser::ActiveJiffies(Pid())) /
         LinuxParser::Jiffies();
}

/**
 * @brief Returns the command that generated this process
 *
 * @return string
 */
string Process::Command() const {
  string command = LinuxParser::Command(Pid());
  if (command.size() > 40) {
    command = command.substr(0, 40) + "...";
  }
  return command;
}

/**
 * @brief Returns this process's memory utilization
 *
 * @return string
 */
string Process::Ram() const { return LinuxParser::Ram(Pid()); }

/**
 * @brief Returns the user (name) that generated this process
 *
 * @return string
 */
string Process::User() const { return LinuxParser::User(Pid()); }

/**
 * @brief Returns the age of this process (in seconds)
 *
 * @return long int
 */
long int Process::UpTime() const { return LinuxParser::UpTime(Pid()); }

/**
 * @brief Overloads the "less than" comparison operator for Process objects
 *
 * @param a
 * @return true
 * @return false
 */
bool Process::operator<(Process const& a) const {
  return std::stol(Ram()) < std::stol(a.Ram());
}
