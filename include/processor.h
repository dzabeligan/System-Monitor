#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();

 private:
  unsigned long activeTime_{0};
  unsigned long idleTime_{0};
};

#endif
