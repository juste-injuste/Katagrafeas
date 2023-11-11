#include "Katagrafeas.hpp"
#include "SomeLibrary.hpp"
#include <chrono>

// scuffed function just to pass time
void sleep(std::chrono::milliseconds::rep ms)
{
  using namespace std::chrono;
  const high_resolution_clock::time_point start = high_resolution_clock::now();
  while (duration_cast<milliseconds>(std::chrono::high_resolution_clock::now() - start).count() < ms);
}

// global logger
Katagrafeas::Stream logger(std::cout, "[Logger %H:%M:%S] ");

int main()
{
  logger.link(SomeLibrary::out, "[SomeLibrary] ");
  logger.link(std::cout, "[std::cout] ");
  logger.link(std::clog, "[std::clog at %H:%M:%S] ");

  SomeLibrary::print("captured output");

  sleep(1000);
  logger << "GLOBAL MESSAGE" << std::endl;

  sleep(1000);
  std::cout << "output on cout" << std::endl;

  sleep(1000);
  std::clog << "ERROR!!!!" << std::endl;

  sleep(2000);
  SomeLibrary::print("other captured output");

  KATAGRAFEAS_ILOG("test %d", 1);
  {
    KATAGRAFEAS_ILOG("test %d", 2);
    {
      KATAGRAFEAS_ILOG("test %d", 3);
      KATAGRAFEAS_ILOG("test %d", 4);
      {
        KATAGRAFEAS_ILOG("test %d", 5);
      }
    }
  }
}
