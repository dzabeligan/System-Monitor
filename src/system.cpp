#include "system.h"

#include <unistd.h>

#include <cstddef>
#include <set>
#include <string>
#include <vector>

#include "linux_parser.h"
#include "process.h"
#include "processor.h"

using namespace std;

/**
 * @brief Returns the system's CPU
 *
 * @return Processor&
 */
Processor& System::Cpu() { return cpu_; }

/**
 * @brief Returns a container composed of the system's processes
 *
 * @return vector<Process>&
 */
vector<Process>& System::Processes() {
  vector<int> pids = LinuxParser::Pids();
  for (const int& pid : pids) {
    auto exists =
        std::find_if(processes_.begin(), processes_.end(),
                     [&pid](Process& process) { return process.Pid() == pid; });
    if (exists == processes_.end()) {
      processes_.emplace_back(pid);
    }
  }
  std::sort(processes_.rbegin(), processes_.rend());
  return processes_;
}

/**
 * @brief Returns the system's kernel identifier (string)
 *
 * @return std::string
 */
std::string System::Kernel() { return LinuxParser::Kernel(); }

/**
 * @brief Returns the system's memory utilization
 *
 * @return float
 */
float System::MemoryUtilization() { return LinuxParser::MemoryUtilization(); }

/**
 * @brief Returns the operating system name
 *
 * @return std::string
 */
std::string System::OperatingSystem() { return LinuxParser::OperatingSystem(); }

/**
 * @brief Returns the number of processes actively running on the system
 *
 * @return int
 */
int System::RunningProcesses() { return LinuxParser::RunningProcesses(); }

/**
 * @brief Returns the total number of processes on the system
 *
 * @return int
 */
int System::TotalProcesses() { return LinuxParser::TotalProcesses(); }

/**
 * @brief Returns the number of seconds since the system started running
 *
 * @return long int
 */
long int System::UpTime() { return LinuxParser::UpTime(); }
