#include "include/Katagrafeas.hpp"
#include <iostream>
#include <fstream>

namespace SomeLibrary
{
  std::ostream out_stream(std::cout.rdbuf());
  std::ostream err_stream(std::cerr.rdbuf());
  std::ostream wrn_stream(std::cerr.rdbuf());

  auto out_print = [](const char* string){out_stream << string << std::endl;};
  auto err_print = [](const char* string){err_stream << string << std::endl;};
  auto wrn_print = [](const char* string){wrn_stream << string << std::endl;};
}

int main()
{
    using namespace Katagrafeas;

    std::ofstream err_file("error_log.txt");
    std::ofstream wrn_file("warning_log.txt");

    Stream out_logger(std::cout);
    Stream wrn_logger(wrn_file, "[WARNING] ");
    Stream err_logger(err_file, "[ERROR] ", " [DATETIME]");

    out_logger.link(SomeLibrary::out_stream);  
    wrn_logger.link(SomeLibrary::wrn_stream);
    err_logger.link(SomeLibrary::err_stream);
    
    SomeLibrary::out_print("text text text");
    SomeLibrary::wrn_print("warning: SomeLibrary: you're wrong, i fixed it.");
    SomeLibrary::err_print("error: SomeLibrary: you broke me lol");

    out_logger << "test_1" << std::endl;
    wrn_logger << "test_2" << std::endl;
    err_logger << "test_3" << std::endl;
}