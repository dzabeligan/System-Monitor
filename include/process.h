#ifndef PROCESS_H
#define PROCESS_H

#include <string>
/*
Basic class for Process representation
It contains relevant attributes as shown below
*/
class Process {
 public:
  explicit Process(int pid) : pid_(pid) {}
  int Pid() const;
  std::string User() const;
  std::string Command() const;
  float CpuUtilization();
  std::string Ram() const;
  long int UpTime() const;
  bool operator<(Process const& a) const;

 private:
  int pid_;
};

#endif
