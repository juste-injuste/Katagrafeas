#include <iostream>

namespace SomeLibrary
{
  std::ostream out{std::cout.rdbuf()};

  void print(const char* text)
  {
    out << text << std::endl;
  }
}