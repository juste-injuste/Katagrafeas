/*---author----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Katagrafeas

-----liscence--------------------------------------------------------------------------------------
 
MIT License

Copyright (c) 2023 Justin Asselin (juste-injuste)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
 
-----versions--------------------------------------------------------------------------------------

Version 1.0.0 - Initial release

-----description-----------------------------------------------------------------------------------

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect
streams towards a common destination. See the included README.MD file for more information.

-----inclusion guard-----------------------------------------------------------------------------*/
#ifndef KATAGRAFEAS_HPP
#define KATAGRAFEAS_HPP
//---necessary standard libraries------------------------------------------------------------------
#include <iosfwd>
#include <iostream>
#include <vector>
//---Katagrafeas library---------------------------------------------------------------------------
namespace Katagrafeas
{
  // library version
  #define KATAGRAFEAS_VERSION       001000000L
  #define KATAGRAFEAS_VERSION_MAJOR 1
  #define KATAGRAFEAS_VERSION_MINOR 0
  #define KATAGRAFEAS_VERSION_PATCH 0
//---Katagrafeas library: frontend forward declarations--------------------------------------------
  inline namespace Frontend
  {
    // default logger
    struct Logger;

    // create logger with custom stream names
    #define KATAGRAFEAS_MAKELOGGER(...)
  }
//---Katagrafeas library: backend struct and class definitions-------------------------------------
  namespace Backend
  {
    class Stream final {
      public:
        inline Stream(std::ostream& stream) noexcept;
        // output to stream directly
        template<typename T>
        inline const Stream& operator<<(const T& text) const noexcept;
        // redirect stream (and backup its original buffer)
        inline void link(std::ostream& stream) const noexcept;
        // restore stream's original buffer
        inline void restore(std::ostream& stream) const noexcept;
      private:
        struct Backup {
          std::ostream* stream;
          std::streambuf* buffer;
        };
        // streams and their original buffer
        mutable std::vector<Backup> backups_;
        // output stream
        std::ostream& stream_;
    };
  }
//---Katagrafeas library: frontend definitions-----------------------------------------------------
  inline namespace Frontend
  {
    struct Logger final {
      inline Logger(std::ostream& out = std::cout,
                    std::ostream& err = std::cerr,
                    std::ostream& wrn = std::cerr,
                    std::ostream& log = std::clog,
                    std::ostream& dbg = std::clog) noexcept;
      // output stream
      const Backend::Stream out;
      // error stream
      const Backend::Stream err;
      // warning stream
      const Backend::Stream wrn;
      // general log stream
      const Backend::Stream log;
      // debug stream
      const Backend::Stream dbg;
    };

    Logger::Logger(std::ostream& out, std::ostream& err, std::ostream& wrn,
                   std::ostream& log, std::ostream& dbg) noexcept
      : // member initialization list
      out(out),
      err(err),
      wrn(wrn),
      log(log),
      dbg(dbg)
    {}

    #undef  KATAGRAFEAS_MAKELOGGER
    #define KATAGRAFEAS_MAKELOGGER(...)      \
      []{                                    \
        struct Logger {                      \
          const Backend::Stream __VA_ARGS__; \
        } logger;                            \
        return logger;                       \
      }()
  }
//---Katagrafeas library: backend struct and class member definitions------------------------------
  namespace Backend
  {
    Stream::Stream(std::ostream& stream) noexcept
      : // member initialization list
      stream_(stream)
    {}

    template<typename T>
    const Stream& Stream::operator<<(const T& text) const noexcept
    {
      // output to stream
      stream_ << text;
      
      return *this;
    }

    void Stream::link(std::ostream& stream) const noexcept
    {
      // backup stream 
      backups_.push_back(Backup{&stream, stream.rdbuf()});

      // redirect stream
      stream.rdbuf(stream_.rdbuf());
    }

    void Stream::restore(std::ostream& stream) const noexcept
    {
      // traverse backups backwards
      for(size_t i = backups_.size() - 1; i < backups_.size(); --i) {
        // find stream in backups
        if (backups_[i].stream == &stream) {
          // restore buffer
          stream.rdbuf(backups_[i].buffer);

          // remove from backup list
          backups_.erase(backups_.begin() + i);

          // stream was restored, no more work is necessary
          return;
        }
      }

      // if we end up here it's because the stream is not part of the backup list
      std::cerr << "error: Stream: couldn't restore ostream; ostream not found in backups\n";
    }
  }
}
#endif
