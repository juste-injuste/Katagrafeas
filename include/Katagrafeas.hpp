// --author------------------------------------------------------------------------------
// 
// Justin Asselin (juste-injuste)
// justin.asselin@usherbrooke.ca
// https://github.com/juste-injuste/Katagrafeas
// 
// --liscence----------------------------------------------------------------------------
// 
// MIT License
// 
// Copyright (c) 2023 Justin Asselin (juste-injuste)
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//  
// --versions----------------------------------------------------------------------------
//
// version 1.0 initial release
//
// --description-------------------------------------------------------------------------
//
// --inclusion guard---------------------------------------------------------------------
#ifndef KATAGRAFEAS_HPP
#define KATAGRAFEAS_HPP
// --necessary standard libraries--------------------------------------------------------
#include <iosfwd>
#include <iostream>
#include <fstream>
// --Katagrafeas library: backend forward declaration------------------------------------
namespace Katagrafeas { namespace Backend
{
  // used to interface with output streams
  class Stream;

  // default logger
  struct Logger;
}}
// --Katagrafeas library: frontend forward declarations----------------------------------
namespace Katagrafeas { inline namespace Frontend
{
  // default logger
  using Backend::Logger;

  // create custom logger
  #define KATAGRAFEAS_MAKELOGGER(...)
}}
// --Katagrafeas library: backend struct and class definitions---------------------------
namespace Katagrafeas { namespace Backend
{
  class Stream final {
    public:
      inline Stream(std::ostream& ostream = std::cout, const char* preamble = "") noexcept;
      inline Stream(std::ofstream& ofstream, const char* preamble = "") noexcept;
      template<typename T>
      inline Stream& operator<<(const T& text) const noexcept;
    private:
      std::ostream* stream;
      const char* preamble;
  };

  struct Logger final {
    inline Logger(void) noexcept;
    inline Logger(Stream out, Stream err, Stream wrn, Stream log) noexcept;
    const Stream out = {std::cout, ""};
    const Stream err = {std::cerr, "error: "};
    const Stream wrn = {std::cerr, "warning: "};
    const Stream log = {std::clog, "log: "};
  };
}}
// --Katagrafeas library: backend struct and class member definitions--------------------
namespace Katagrafeas { namespace Backend
{
  Stream::Stream(std::ostream& ostream, const char* preamble) noexcept
    : // member initialization list
    stream(&ostream),
    preamble(preamble)
  {}

  Stream::Stream(std::ofstream& ofstream, const char* preamble) noexcept
    : // member initialization list
    stream(&ofstream),
    preamble(preamble)
  {}

  template<typename T>
  Stream& Stream::operator<<(const T& text) const noexcept
  {
    stream->operator<<(preamble);
    
    stream->operator<<(text);

    return *this;
  }

  Logger::Logger(void) noexcept
  {}

  Logger::Logger(Stream out, Stream err, Stream wrn, Stream log) noexcept
    : // member initialization list
    out(out),
    err(err),
    wrn(wrn),
    log(log)
  {}
}}
// --Katagrafeas library: frontend definitions-------------------------------------------
namespace Katagrafeas { inline namespace Frontend
{
  #undef  KATAGRAFEAS_MAKELOGGER
  #define KATAGRAFEAS_MAKELOGGER(...)       \
    []{                                     \
      using namespace Katagrafeas::Backend; \
      struct Logger {                       \
        const Stream __VA_ARGS__;           \
      } logger;                             \
      return logger;                        \
    }()
}}
#endif

int main()
{
  using namespace Katagrafeas;

  Logger test;
  test.err << "test";
}
