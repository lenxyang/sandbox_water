#ifndef TIMER_H
#define TIMER_H

#include "base/time/time.h"

class cTimer {
 private:
  ::base::TimeTicks process_start, frame_start, current;
 protected:
 public:
  cTimer();
  ~cTimer();
  double elapsed(bool frame);
};

#endif
