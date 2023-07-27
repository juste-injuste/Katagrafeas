#include <iostream>
#include <chrono>
#include <ctime>
#include "include/Katagrafeas.hpp"
#include "testing/Chronometros.hpp"
#include <iomanip>

int main() {
    Chronometro::Stopwatch<> sw;
    
    sw.reset();
    std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    std::stringstream time_stream;
    std::cout << std::put_time(std::localtime(&time), "other datatime: %y-%m-%d %H:%M:%S") << std::endl;
    sw.split();
}
