#include "timer.h"

#include <iostream>
#include <cassert>

Timer::Timer()
{
  start();
}

void
Timer::start()
{
  startTime = getTicks();
}

uint64_t
Timer::getTicks()
{
  uint64_t ticks;
  struct timeval now;
  gettimeofday(&now, NULL);
  ticks = (now.tv_sec)*1000000+now.tv_usec;
  return ticks;
}

uint64_t
Timer::getElapsed()
{
  return getTicks()-startTime;
}
