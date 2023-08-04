/*---author-----------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Katagrafeas

-----liscence---------------------------------------------------------------------------------------
 
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
 
-----versions---------------------------------------------------------------------------------------

Version 1.0.0 - Initial release

-----description------------------------------------------------------------------------------------

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect
streams towards a common destination. See the included README.MD file for more information.

-----inclusion guard------------------------------------------------------------------------------*/
#ifndef KATAGRAFEAS_HPP
#define KATAGRAFEAS_HPP
//---necessary standard libraries-------------------------------------------------------------------
#include <ostream>    // for std::ostream
#include <streambuf>  // for std::streambuf
#include <iostream>   // for std::cerr
#include <vector>     // for std::vector
#include <cstddef>    // for size_t
#include <string>     // for std::string
#include <memory>     // for std::unique_ptr
#include <functional> // for std::function
#include <utility>    // for std::pair
#include <chrono>
#include <sstream>
#include <iomanip>    // for std::put_time
#include <array>
#include <ctime>
#include <cstring>
//---Katagrafeas library----------------------------------------------------------------------------
namespace Katagrafeas
{
  namespace Version
  {
    constexpr long MAJOR  = 000;
    constexpr long MINOR  = 001;
    constexpr long PATCH  = 000;
    constexpr long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Katagrafeas library: frontend forward declarations---------------------------------------------
  inline namespace Frontend
  {
    // ostream redirection aswell as prefixing and suffixing
    class Stream;
  }
//---Katagrafeas library: backend forward declarations----------------------------------------------
  namespace Backend
  {
    // intercept individual ostreams to use a unique prefix and suffix
    class Interceptor;

    // return type of std::put_time
    using put_time_t = decltype(std::put_time(nullptr, ""));

    // return formated
    inline put_time_t format_string(const char* format);
  }
// --Katagrafeas library: backend struct and class definitions--------------------------------------
  namespace Backend
  {
    class Interceptor final : public std::streambuf {
      private:
        inline Interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept;
        // associated Stream instance
        Stream* const stream;
        // kept track of so it can be restored later
        std::ostream* const redirected_ostream;
        // buffer of redirected_ostream before it was redirected
        std::streambuf* const original_buffer;
        // prefix for new messages
        const std::string prefix_;
        // suffix for newlines
        const std::string suffix_;
      protected:
        // intercept character and forward it to streambuf
        inline virtual int_type overflow(int_type c) override;
        // intercept sync and forward it to Stream
        inline virtual int sync() override;
      friend class Frontend::Stream;
    };
  }
// --Katagrafeas library: frontend struct and class definitions-------------------------------------
  inline namespace Frontend
  {
    class Stream final {
      public:
        inline Stream(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
        // restore all ostreams orginal buffer
        inline ~Stream() noexcept;
        // redirect ostream (and backup its original buffer)
        void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
        // restore ostream's original buffer
        void restore(std::ostream& ostream) noexcept;
        // restore all ostreams orginal buffer
        void restore_all(void) noexcept;
        // output to ostream
        template<typename T>
        inline std::ostream& operator<<(const T& text) noexcept;
        // apply manipulator to ostream
        inline std::ostream& operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept;
      private:
        // store intercepted ostreams
        std::vector<std::unique_ptr<Backend::Interceptor>> backups_;
        // output streambuf
        std::streambuf* buffer_;
        // underlying ostream linked to the output buffer
        std::ostream ostream_;
        // prefix for new messages
        const char* prefix_;
        // suffix for newlines
        const char* suffix_;
        // prepending flag
        bool prepend_;
      friend class Backend::Interceptor;
    };
  }
// --Katagrafeas library: frontend struct and class member definitions------------------------------
  inline namespace Frontend
  {
    Stream::Stream(std::ostream& ostream, const char* prefix, const char* suffix) noexcept
      : // member initialization list
      buffer_(ostream.rdbuf()),
      ostream_(buffer_),
      prefix_(prefix),
      suffix_(suffix),
      prepend_(true)
    {}

    Stream::~Stream() noexcept
    {
      restore_all();
    }

    void Stream::link(std::ostream& ostream, const char* prefix, const char* suffix) noexcept
    {
      // create interceptor
      Backend::Interceptor* interceptor = new Backend::Interceptor(this, ostream, prefix, suffix);

      // store and take interceptor ownership via std::unique_ptr
      backups_.emplace_back(interceptor);

      // redirect towards the interceptor
      ostream.rdbuf(interceptor);
    }

    void Stream::restore(std::ostream& ostream) noexcept
    {
      // traverse backups backwards
      for(size_t i = backups_.size() - 1; i < backups_.size(); --i) {
        // find ostream in backups
        if (backups_[i]->redirected_ostream == &ostream) {
          // restore streambuf
          ostream.rdbuf(backups_[i]->original_buffer);

          // remove from backup list
          backups_.erase(backups_.begin() + i);

          // stream was restored, no more work is necessary
          return;
        }
      }

      // if we end up here it's because the ostream is not part of the backup list
      std::cerr << "error: Stream: couldn't restore ostream; ostream not found in backups\n";
    }

    void Stream::restore_all(void) noexcept
    {
      // restore all ostreams
      for(std::unique_ptr<Backend::Interceptor>& backup : backups_)
        backup->redirected_ostream->rdbuf(backup->original_buffer);
    }

    template<typename T>
    std::ostream& Stream::operator<<(const T& text) noexcept
    {
      // output to stream
      return ostream_ << Backend::format_string(prefix_) << text;
    }

    std::ostream& Stream::operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept
    {
      // apply manipulator
      return manipulator(ostream_);
    }
  }
// --Katagrafeas library: backend struct and class member definitions-------------------------------
  namespace Backend
  {    
    Interceptor::Interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept
      : // member initialization list
      stream(stream),
      redirected_ostream(&ostream),
      original_buffer(ostream.rdbuf()),
      prefix_(prefix),
      suffix_(suffix)
    {}

    Interceptor::int_type Interceptor::overflow(int_type c)
    {
      if (stream->prepend_) {
        stream->prepend_ = false;
        stream->ostream_ << Backend::format_string(stream->prefix_) << prefix_;
      }
      else if (c == '\n') {
        stream->ostream_ << suffix_ << Backend::format_string(stream->suffix_);
      }

      return stream->buffer_->sputc(c);
    }

    int Interceptor::sync()
    {
      stream->prepend_ = true;
      return stream->buffer_->pubsync();
    }

    put_time_t format_string(const char* format)
    {
      std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format);
    }
  }
}
#endif