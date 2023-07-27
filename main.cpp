#include <iostream>
#include <chrono>
#include <ctime>
#include "include/Katagrafeas.hpp"
#include "testing/Chronometros.hpp"

int main() {
    auto test = Katagrafeas::Backend::get_datatime_parser("YY-MM-DD hh:mm:ss");

    Chronometro::Stopwatch<> sw;
    test();
    sw.split();
}