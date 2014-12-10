#include "timer.h"

cTimer::cTimer() {
  process_start = ::base::TimeTicks::HighResNow();;
  frame_start = process_start;
}

cTimer::~cTimer() {
	
}

double cTimer::elapsed(bool frame) {
  ::base::TimeTicks now = ::base::TimeTicks::HighResNow();
  ::base::TimeDelta delta = now - frame_start;
  frame_start = now;
  return delta.InSecondsF();
}
