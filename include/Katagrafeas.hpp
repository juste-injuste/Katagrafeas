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
#include <vector>
// --Katagrafeas library-----------------------------------------------------------------
namespace Katagrafeas {
  // library version
  #define KATAGRAFEAS_VERSION       001000000L
  #define KATAGRAFEAS_VERSION_MAJOR 1
  #define KATAGRAFEAS_VERSION_MINOR 0
  #define KATAGRAFEAS_VERSION_PATCH 0
// --Katagrafeas library: frontend forward declarations----------------------------------
  inline namespace Frontend
  {
    // default logger
    struct Logger;

    // create logger with custom stream names
    #define KATAGRAFEAS_MAKELOGGER(...)
  }
// --Katagrafeas library: backend struct and class definitions---------------------------
  namespace Backend
  {
    class Stream final {
      public:
        inline Stream(std::ostream& stream) noexcept;
        template<typename T>
        inline std::ostream& operator<<(const T& text) const noexcept;
        // redirect stream (and save its original buffer)
        inline void redirect(std::ostream& stream) const noexcept;
        // restore stream's original buffer
        inline void restore(std::ostream& stream) const noexcept;
      private:
        struct Backup {
          std::ostream* stream;
          std::streambuf* buffer;
        };
        // streams and their original buffer
        mutable std::vector<Backup> backups_;
        // destination stream
        std::ostream& stream_;
    };
  }
// --Katagrafeas library: frontend struct and class definitions--------------------------
  inline namespace Frontend
  {
    struct Logger final {
      inline Logger(void) noexcept;
      inline Logger(std::ostream& out, std::ostream& err, std::ostream& wrn, std::ostream& log) noexcept;
      const Backend::Stream out;
      const Backend::Stream err;
      const Backend::Stream wrn;
      const Backend::Stream log;
    };
  }
// --Katagrafeas library: frontend struct and class member definitions-------------------
  inline namespace Frontend
  {
    Logger::Logger(void) noexcept
      : // member initialization list
      out(std::cout),
      err(std::cerr),
      wrn(std::cerr),
      log(std::clog)
    {}

    Logger::Logger(std::ostream& out, std::ostream& err, std::ostream& wrn, std::ostream& log) noexcept
      : // member initialization list
      out(out),
      err(err),
      wrn(wrn),
      log(log)
    {}
  }
// --Katagrafeas library: frontend definitions-------------------------------------------
  inline namespace Frontend
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
  }
// --Katagrafeas library: backend struct and class member definitions--------------------
  namespace Backend
  {
    Stream::Stream(std::ostream& stream) noexcept
      : // member initialization list
      stream_(stream)
    {}

    template<typename T>
    std::ostream& Stream::operator<<(const T& text) const noexcept
    {
      // output to stream
      return stream_ << text;
    }

    void Stream::redirect(std::ostream& stream) const noexcept
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
        // find stream in list
        if (backups_[i].stream == &stream) {
          // restore buffer
          stream.rdbuf(backups_[i].buffer);

          // remove from backup list
          backups_.erase(backups_.begin() + i);

          return;
        }
      }
      
      // if we end up here it's because the stream is not part of the backup list
      std::cerr << "error: Stream: couldn't restore stream; stream not found in backups\n";
    }
  }
}
#endif
