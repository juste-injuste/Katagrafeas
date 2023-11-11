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
#include <ostream>   // for std::ostream
#include <streambuf> // for std::streambuf
#include <cstddef>   // for size_t
#include <vector>    // for std::vector
#include <memory>    // for std::unique_ptr
#include <chrono>    // for std::chrono::system_clock::now, std::chrono::system_clock::to_time_t
#include <iomanip>   // for std::put_time
#include <ctime>     // for std::localtime, std::time_t
#include <iostream>  // for std::clog, std::cerr
#include <sstream>   // for std::stringstream
#include <cstdio>    // for std::vsnprintf
#include <cstdarg>
//---Katagrafeas library----------------------------------------------------------------------------
namespace Katagrafeas
{
  namespace Version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  namespace Global
  {
    std::ostream log{std::clog.rdbuf()};
    std::ostream wrn{std::cerr.rdbuf()};
  }
//---Katagrafeas library: frontend forward declarations---------------------------------------------
  inline namespace Frontend
  {
    // ostream redirection aswell as prefixing and suffixing
    class Stream;
    
    // log a message
    #define KATAGRAFEAS_LOG(message)

    // log a message using stack-based indentation
    #define KATAGRAFEAS_ILOG(message)

    // issue a warning
    #define KATAGRAFEAS_WARNING(message)

    // issue an error along with a code
    #define KATAGRAFEAS_ERROR(message, code)
  }
//---Katagrafeas library: backend forward declarations----------------------------------------------
  namespace Backend
  {
    // intercept individual ostreams to use a unique prefix and suffix
    class Interceptor final : public std::streambuf
    {
      public:
        inline Interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept;
        // restore original buffer
        inline ~Interceptor() noexcept;
        // kept track of so it can be restored later
        std::ostream* const redirected_ostream;
      private:
        // associated Stream instance
        Stream* const stream;
        // buffer of redirected_ostream before it was redirected
        std::streambuf* const original_buffer;
        // prefix for new messagess
        const char* prefix;
        // suffix for newlines
        const char* suffix;
        // intercept character and forward it to streambuf
        inline virtual int_type overflow(int_type character) override;
        // intercept sync and forward it to Stream
        inline virtual int sync() override;
    };

    // return type of std::put_time
    using Formatted = decltype(std::put_time((const std::tm*) nullptr, (const char*) nullptr));

    // return time formatted string
    inline Formatted format_string(const char* format);

    class IndentedLog
    {
    public:
      inline IndentedLog(const std::string& text, const std::string& caller = "") noexcept
      {
        Global::log << "log: " << caller;

        for (size_t k = indentation; k; --k)
        {
          Global::log << ' ';
        }

        Global::log << text << std::endl;

        indentation += indentation_size;
      }

      inline ~IndentedLog() noexcept
      {
        indentation -= indentation_size;
      }
    private:
      static unsigned indentation;
      static unsigned constexpr indentation_size = 2;
    };
    unsigned IndentedLog::indentation;
  }
// --Katagrafeas library: frontend struct and class definitions-------------------------------------
  inline namespace Frontend
  {
    class Stream final
    {
      public:
        inline Stream(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
        // restore all ostreams orginal buffer
        inline ~Stream() noexcept;
        // redirect ostream (and backup its original buffer)
        void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
        // restore ostream's original buffer
        bool restore(std::ostream& ostream) noexcept;
        // restore all ostreams orginal buffer
        inline void restore_all(void) noexcept;
        // interect with general ostream
        template<typename T>
        inline std::ostream& operator<<(const T& anything) noexcept;
      private:
        // store intercepted ostreams
        std::vector<std::unique_ptr<Backend::Interceptor>> backups;
        // output streambuf
        std::streambuf* buffer;
        // underlying ostream linked to the output buffer
        std::ostream underlying_ostream{buffer};
        // ostream for general io
        std::ostream general_ostream{nullptr};
        // prefix for new messages
        const char* prefix;
        // suffix for newlines
        const char* suffix;
        // prepending flag
        bool prepend_flag = true;
      friend class Backend::Interceptor;
    };

#   undef  KATAGRAFEAS_LOG
#   define KATAGRAFEAS_LOG(text)  Global::log << "log: " << __func__ << ": " << text << std::endl;

#   undef  KATAGRAFEAS_ILOG
#   define KATAGRAFEAS_ILOG_IMPL(line, text, ...)     \
      Katagrafeas::Backend::IndentedLog ilog_##line { \
          [&]{                                        \
              char buffer[1024];                      \
              sprintf(buffer, text, __VA_ARGS__);     \
              return std::string(buffer);             \
          }(), __func__};
#   define KATAGRAFEAS_ILOG_TEMP(line, text, ...) KATAGRAFEAS_ILOG_IMPL(line, text, __VA_ARGS__)
#   define KATAGRAFEAS_ILOG(text, ...)            KATAGRAFEAS_ILOG_TEMP(__LINE__, text, __VA_ARGS__)

#   undef  KATAGRAFEAS_WARNING
#   define KATAGRAFEAS_WARNING(message)

#   undef  KATAGRAFEAS_ERROR
#   define KATAGRAFEAS_ERROR(message, code)
  }
// --Katagrafeas library: frontend struct and class member definitions------------------------------
  inline namespace Frontend
  {
    Stream::Stream(std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
      buffer{ostream.rdbuf()},
      prefix{prefix},
      suffix{suffix}
    {
      link(general_ostream);
    }

    Stream::~Stream() noexcept
    {
      restore_all();
    }

    void Stream::link(std::ostream& ostream, const char* prefix, const char* suffix) noexcept
    {
      // construct and store interceptor unique pointer
      backups.emplace_back(new Backend::Interceptor(this, ostream, prefix, suffix));

      // redirect towards the new interceptor
      ostream.rdbuf(backups.back().get());
    }

    bool Stream::restore(std::ostream& ostream) noexcept
    {
      // look for ostream in backups
      for (size_t k = backups.size() - 1; k; --k)
      {
        if (backups[k]->redirected_ostream == &ostream)
        {
          // delete from backup list (destructor restores buffer)
          backups.erase(backups.begin() + k);

          // stream was restored, no more work is necessary
          return true;
        }
      }

      Global::wrn << "warning: Stream::restore(std::ostream&): ostream was not in in backup list" << std::endl;

      // ostream is not part of the backup list
      return false;
    }

    void Stream::restore_all(void) noexcept
    {
      // restore all ostreams (destructor restores buffer)
      backups.clear();
    }

    template<typename T>
    std::ostream& Stream::operator<<(const T& anything) noexcept
    {
      return general_ostream << anything;
    }
  }
// --Katagrafeas library: backend struct and class member definitions-------------------------------
  namespace Backend
  {
    Interceptor::Interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
      redirected_ostream{&ostream},
      stream{stream},
      original_buffer{ostream.rdbuf()},
      prefix{prefix},
      suffix{suffix}
    {}

    Interceptor::~Interceptor() noexcept
    {
      redirected_ostream->rdbuf(original_buffer);
    }

    Interceptor::int_type Interceptor::overflow(int_type character)
    {
      if (stream->prepend_flag)
      {
        stream->prepend_flag = false;
        stream->underlying_ostream << Backend::format_string(stream->prefix);
        stream->underlying_ostream << Backend::format_string(prefix);
      }
      else if (character == '\n')
      {
        stream->underlying_ostream << Backend::format_string(suffix);
        stream->underlying_ostream << Backend::format_string(stream->suffix);
      }

      return stream->buffer->sputc(character);
    }

    int Interceptor::sync()
    {
      stream->prepend_flag = true;
      return stream->buffer->pubsync();
    }

    Formatted format_string(const char* format)
    {
      std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format);
    }
  }
}
#endif