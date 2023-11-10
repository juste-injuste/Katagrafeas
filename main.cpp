#include <iostream>
#include <chrono>
#include <ctime>
#include "include/Katagrafeas.hpp"
#include "testing/Chronometro.hpp"
#include <iomanip>

// scuffed sleep function to demonstrate the basic usage of the library
void sleep_for_ms(std::chrono::high_resolution_clock::rep ms)
{
  auto start = std::chrono::high_resolution_clock::now();
  while(std::chrono::nanoseconds(std::chrono::high_resolution_clock::now()-start).count() < ms*1000000);
}

Katagrafeas::Stream logger(std::cout, "[Logger %H:%M:%S] ");
Chronometro::Stopwatch<> stopwatch;

int main()
{
    logger.link(Chronometro::Global::out, "[Chronometro] ");

    stopwatch.reset();
      sleep_for_ms(1000);
    stopwatch.lap();
      sleep_for_ms(2000);
    stopwatch.split();
}
