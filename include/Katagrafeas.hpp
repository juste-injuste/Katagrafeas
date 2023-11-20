/*---author-------------------------------------------------------------------------------------------------------------

Justin Asselin (juste-injuste)
justin.asselin@usherbrooke.ca
https://github.com/juste-injuste/Katagrafeas

-----liscence-----------------------------------------------------------------------------------------------------------
 
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
 
-----versions-----------------------------------------------------------------------------------------------------------

Version 1.0.0 - Initial release

-----description--------------------------------------------------------------------------------------------------------

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect
streams towards a common destination. See the included README.MD file for more information.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef KATAGRAFEAS_HPP
#define KATAGRAFEAS_HPP
//---necessary standard libraries---------------------------------------------------------------------------------------
#include <ostream>   // for std::ostream
#include <streambuf> // for std::streambuf
#include <cstddef>   // for size_t
#include <vector>    // for std::vector
#include <memory>    // for std::unique_ptr
#include <chrono>    // for std::chrono::system_clock::now, std::chrono::system_clock::to_time_t
#include <iomanip>   // for std::put_time
#include <ctime>     // for std::localtime, std::time_t
#include <iostream>  // for std::clog, std::cerr
#include <cstdio>    // for std::sprintf
//---Katagrafeas library------------------------------------------------------------------------------------------------
namespace Katagrafeas
{
  // ostream redirection aswell as prefixing and suffixing
  class Stream;
  
  // log a message
# define KATAGRAFEAS_LOG(message)

  // log a message using stack-based indentation
# define KATAGRAFEAS_ILOG(message)

  // issue a warning
# define KATAGRAFEAS_WARNING(message)

  // issue an error along with a code
# define KATAGRAFEAS_ERROR(message, code)

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
    std::ostream wrn{std::clog.rdbuf()};
    std::ostream err{std::cerr.rdbuf()};
  }
//---Katagrafeas library: backend forward declarations------------------------------------------------------------------
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
        std::ostream* const ostream;
      private:
        // associated Stream instance
        Stream* const stream;
        // ostream's buffer before it's redirected by *stream
        std::streambuf* const buffer_backup = ostream->rdbuf();
        // prefix for new messagess
        const char* prefix;
        // suffix for newlines
        const char* suffix;
        // intercept character and forward it to streambuf
        inline virtual int_type overflow(int_type character) override;
        // intercept sync and forward it to Stream
        inline virtual int sync() override;
    };

    class IndentedLog final
    {
    public:
      inline IndentedLog(const char* text, const char* caller = "") noexcept;
      inline ~IndentedLog() noexcept;
    private:
      static unsigned indentation;
      static unsigned constexpr indentation_size = 2;
    };

    // return type of std::put_time
    using Formatted = decltype(std::put_time((const std::tm*) nullptr, (const char*) nullptr));

    // return time formatted string
    inline Formatted format_string(const char* format);

# if not defined(KATAGRAFEAS_MAX_LEN)
#   define KATAGRAFEAS_MAX_LEN 256
# endif
  }
// --Katagrafeas library: frontend struct and class definitions---------------------------------------------------------
  class Stream final
  {
  public:
    inline explicit Stream(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    // restore all ostreams orginal buffer
    inline ~Stream() noexcept;
    // redirect ostream (and backup its original buffer)
    inline void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    // restore ostream's original buffer
    inline bool restore(std::ostream& ostream) noexcept;
    // restore all ostreams orginal buffer
    inline void restore_all() noexcept;
    // interact with general ostream
    template<typename T>
    inline std::ostream& operator<<(const T& anything) noexcept;
  private:
    // store intercepted ostreams
    std::vector<std::unique_ptr<Backend::Interceptor>> backups;
    // output buffer
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

# undef  KATAGRAFEAS_LOG
# define KATAGRAFEAS_LOG_IMPL1(line, message) Katagrafeas::Global::log << "log: " << __func__ << ": " << message << std::endl
# define KATAGRAFEAS_LOG_IMPL2(line, message, ...)                \
    [&](const char* caller){                                      \
      static char formatted_message[KATAGRAFEAS_MAX_LEN];         \
      sprintf(formatted_message, message, __VA_ARGS__);           \
      Katagrafeas::Global::log << "log: " << caller << ": ";      \
      Katagrafeas::Global::log << formatted_message << std::endl; \
    }(__func__)
# define KATAGRAFEAS_LOG_IMPL(_1, _2, _3, NARGS, ...) KATAGRAFEAS_LOG_IMPL##NARGS
# define KATAGRAFEAS_LOG_LINE(...)                    KATAGRAFEAS_LOG_IMPL(__VA_ARGS__, 2, 1, 0)(__VA_ARGS__)
# define KATAGRAFEAS_LOG(...)                         KATAGRAFEAS_LOG_LINE(__LINE__, __VA_ARGS__)

# undef  KATAGRAFEAS_ILOG
# define KATAGRAFEAS_ILOG_IMPL1(line, message) Katagrafeas::Backend::IndentedLog ilog_##line { message, __func__}
# define KATAGRAFEAS_ILOG_IMPL2(line, message, ...) \
    Katagrafeas::Backend::IndentedLog ilog_##line { \
      [&]{                                          \
        static char buffer[KATAGRAFEAS_MAX_LEN];    \
        sprintf(buffer, message, __VA_ARGS__);      \
        return buffer;                              \
      }(), __func__}
# define KATAGRAFEAS_ILOG_IMPL(_1, _2, _3, NARGS, ...) KATAGRAFEAS_ILOG_IMPL##NARGS
# define KATAGRAFEAS_ILOG_LINE(...)                    KATAGRAFEAS_ILOG_IMPL(__VA_ARGS__, 2, 1, 0)(__VA_ARGS__)
# define KATAGRAFEAS_ILOG(...)                         KATAGRAFEAS_ILOG_LINE(__LINE__, __VA_ARGS__)

# undef  KATAGRAFEAS_WARNING
# define KATAGRAFEAS_WARNING(message) Global::wrn << "warning: " << __func__ << ": " << message << std::endl

# undef  KATAGRAFEAS_ERROR
# define KATAGRAFEAS_ERROR(message, code)
// --Katagrafeas library: frontend struct and class member definitions--------------------------------------------------
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
    backups.emplace_back(new Backend::Interceptor(this, ostream, prefix, suffix));
    ostream.rdbuf(backups.back().get()); // redirect towards the new interceptor
  }

  bool Stream::restore(std::ostream& ostream) noexcept
  {
    for (size_t k = backups.size() - 1; k; --k)
    {
      if (backups[k]->ostream == &ostream)
      {
        backups.erase(backups.begin() + k);
        return true;
      }
    }

    Global::wrn << "warning: Stream::restore(std::ostream&): ostream was not in in backup list" << std::endl;
    return false;
  }

  void Stream::restore_all() noexcept
  {
    backups.clear();
  }

  template<typename T>
  std::ostream& Stream::operator<<(const T& anything) noexcept
  {
    return general_ostream << anything;
  }
// --Katagrafeas library: backend struct and class member definitions---------------------------------------------------
  namespace Backend
  {
    Interceptor::Interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
      ostream{&ostream},
      stream{stream},
      prefix{prefix},
      suffix{suffix}
    {}

    Interceptor::~Interceptor() noexcept
    {
      ostream->rdbuf(buffer_backup);
    }

    Interceptor::int_type Interceptor::overflow(int_type character)
    {
      if (character == '\n')
      {
        stream->underlying_ostream << Backend::format_string(suffix);
        stream->underlying_ostream << Backend::format_string(stream->suffix);
      }
      else if (stream->prepend_flag)
      {
        stream->prepend_flag = false;
        stream->underlying_ostream << Backend::format_string(stream->prefix);
        stream->underlying_ostream << Backend::format_string(prefix);
      }

      return stream->buffer->sputc(character);
    }

    int Interceptor::sync()
    {
      stream->prepend_flag = true;
      return 0;
    }
    
    unsigned IndentedLog::indentation = 0;

    IndentedLog::IndentedLog(const char* text, const char* caller) noexcept
    {
      Global::log << "log: " << caller << ": ";

      for (unsigned k = indentation; k; --k)
      {
        Global::log << ' ';
      }

      Global::log << text << std::endl;

      indentation += indentation_size;
    }

    IndentedLog::~IndentedLog() noexcept
    {
      indentation -= indentation_size;
    }

    Formatted format_string(const char* format)
    {
      const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format);
    } 
  }
}
#endif