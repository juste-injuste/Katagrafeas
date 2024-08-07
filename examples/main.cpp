// #include "../include/Katagrafeas.hpp"
// #include <iostream>

// int main()
// {
//   std::cout << "huh??\n";
//   ktz::Logger logger(std::cerr, "[Logger %H:%M:%S] ", "");
//   std::cout << "huh??\n";
// }

#include <iostream>
#include "SomeLibrary.hpp"
#include "../include/Katagrafeas.hpp"
#include "../include/Chronometro.hpp"

// ktz::Logger logger(std::cout);

int main()
{
  // logger.link(SomeLibrary::out, "[SomeLibrary] ");
  // logger.link(std::cout, "[std::cout] ");
  // logger.link(std::cerr, "[std::cerr] ", " [at %H:%M:%S]");

  SomeLibrary::print("captured output");

  chz::sleep(1000);
  // logger << "GLOBAL MESSAGE" << std::endl;

  chz::sleep(1000);
  std::cout << "output on cout" << std::endl;

  chz::sleep(1000);
  std::cerr << "ERROR!!!!" << std::endl;

  chz::sleep(1000);
  SomeLibrary::print("other captured output");

  ktz::indented_log("test %d", 1);
  {
    ktz::indented_log("test %d", 2);
    {
      ktz::indented_log("test %d", 3);
      {
        ktz::indented_log("test %d", 5);
      }
      ktz::indented_log("test %d", 4);
    }
    ktz::indented_log("test %d", 4);
  }
  ktz::indented_log("test %d", 4);

  ktz::indented_log("some message");
  ktz::indented_log("some formatted %s %s", "message", "3");

  ktz::log_message("some message");
  ktz::log_message("some formatted %s, %d", "message", 3);
}