#include "processor.h"

#include <numeric>
#include <vector>

#include "linux_parser.h"

/**
 * @brief Returns the aggregate CPU utilization
 *
 * @return float
 */
float Processor::Utilization() {
  long idleTime = LinuxParser::IdleJiffies();
  long activeTime = LinuxParser::ActiveJiffies();
  long dIdleTime = idleTime - idleTime_;
  long dNonIdleTime = activeTime - activeTime_;

  float percentage =
      static_cast<float>(dNonIdleTime) / (dIdleTime + dNonIdleTime);
  idleTime_ = idleTime;
  activeTime_ = activeTime;

  return percentage;
}
