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
#ifdef __STDCPP_THREADS__
# include <mutex>
#endif
//---Katagrafeas library------------------------------------------------------------------------------------------------
namespace Katagrafeas
{
  namespace Version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }

  // ostream redirection aswell as prefixing and suffixing
  class Stream;

# define KATAGRAFEAS_LOG(message, ...)    // log a formattable message
# define KATAGRAFEAS_ILOG(message, ...)   // log a formattable message using stack-based indentation
# define KATAGRAFEAS_WARNING(message)     // issue a warning
# define KATAGRAFEAS_ERROR(message, code) // issue an error along with a code

  namespace Global
  {
    std::ostream log{std::clog.rdbuf()};
    std::ostream wrn{std::clog.rdbuf()};
    std::ostream err{std::cerr.rdbuf()};
  }

# ifndef KATAGRAFEAS_MAX_LEN
#   define KATAGRAFEAS_MAX_LEN 256
# endif
//---Katagrafeas library: backend forward declarations------------------------------------------------------------------
  namespace _backend
  {
# if defined(__GNUC__) and (__GNUC__ >= 9)
#   define KATAGRAFEAS_COLD [[unlikely]]
#   define KATAGRAFEAS_HOT  [[likely]]
# elif defined(__clang__) and (__clang_major__ >= 9)
#   define KATAGRAFEAS_COLD [[unlikely]]
#   define KATAGRAFEAS_HOT  [[likely]]
# else
#   define KATAGRAFEAS_COLD
#   define KATAGRAFEAS_HOT
# endif

    // intercept individual ostreams to use a unique prefix and suffix
    class _interceptor final : public std::streambuf
    {
    public:
      _interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
        ostream{&ostream},
        stream{stream},
        prefix{prefix},
        suffix{suffix}
      {}
      
      ~_interceptor() noexcept
      {
        ostream->rdbuf(buffer_backup); // restore original buffer
      }

      std::ostream*   const ostream; // kept track of to restore it later
    private:
      std::streambuf* const buffer_backup = ostream->rdbuf();
      Stream* const         stream;  // associated Stream instance
      const char* const     prefix;  // prefix for new messages
      const char* const     suffix;  // suffix for newlines

      inline int_type overflow(int_type character) override;
      
      inline int sync() override;
    };

    // return time formatted string
    auto _format_string(const char* format) -> decltype(std::put_time((const std::tm*)0, (const char*)0))
    {
      const std::time_t time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format);
    }

    class _indentedlog final
    {
    public:
      _indentedlog(const char* text, const char* caller = "") noexcept
      {
        Global::log << "log: " << caller << ": ";

        for (unsigned k = indentation(); k; --k)
        {
          Global::log << ' ';
        }

        Global::log << text << std::endl;

        indentation() += indentation_size;
      }

      ~_indentedlog() noexcept
      {
        indentation() -= indentation_size;
      }
    private:
      static inline unsigned& indentation() noexcept
      {
        static unsigned indentation = 0;
        return indentation;
      }
      static constexpr unsigned indentation_size = 2;
    };
    
    void _log(const char* caller, const char* message)
    {
#   ifdef __STDCPP_THREADS__
      static std::mutex mtx;
      std::lock_guard<std::mutex> lock{mtx};
#   endif
      Global::log << "log: " << caller;
      Global::log << ": " << message << std::endl;
    }
  }
// --Katagrafeas library: frontend struct and class definitions---------------------------------------------------------
  class Stream final
  {
  public:
    inline Stream(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
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
    inline Stream& operator <<(const T& anything) noexcept;
    inline Stream& operator <<(std::ostream& (*manipulator)(std::ostream&)) noexcept;
  private:
    // store intercepted ostreams
    std::vector<std::unique_ptr<_backend::_interceptor>> backups;
    // output buffer
    std::streambuf* buffer;
    // underlying ostream linked to the output buffer
    std::ostream underlying_ostream{buffer};
    // ostream for general io
    std::ostream general_ostream{nullptr};
    // prefix for new messages
    const char* const prefix;
    // suffix for newlines
    const char* const suffix;
    // prepending flag
    bool prepend_flag = true;
  friend class _backend::_interceptor;
  };

# undef  KATAGRAFEAS_LOG
# define KATAGRAFEAS_LOG(...)                    \
    [&](const char* caller){                     \
      static char buffer[KATAGRAFEAS_MAX_LEN];   \
      sprintf(buffer, __VA_ARGS__);              \
      Katagrafeas::_backend::_log(caller, buffer); \
    }(__func__)

# undef  KATAGRAFEAS_ILOG
# define KATAGRAFEAS_ILOG_IMPL(line, ...)           \
    Katagrafeas::_backend::_indentedlog ilog_##line { \
      [&]{                                          \
        static char buffer[KATAGRAFEAS_MAX_LEN];    \
        sprintf(buffer, __VA_ARGS__);               \
        return buffer;                              \
      }(), __func__}
# define KATAGRAFEAS_ILOG_PROX(line, ...) KATAGRAFEAS_ILOG_IMPL(line,     __VA_ARGS__)
# define KATAGRAFEAS_ILOG(...)            KATAGRAFEAS_ILOG_PROX(__LINE__, __VA_ARGS__)

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
    backups.emplace_back(new _backend::_interceptor(this, ostream, prefix, suffix));
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
  Stream& Stream::operator<<(const T& anything) noexcept
  {
    general_ostream << anything;
    return *this;
  }

  Stream& Stream::operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept
  {
    manipulator(general_ostream);
    return *this;
  }
// ---
  namespace _backend
  {
    _interceptor::int_type _interceptor::overflow(int_type character)
    {
      if (character == '\n') KATAGRAFEAS_COLD
      {
        stream->underlying_ostream << _backend::_format_string(suffix);
        stream->underlying_ostream << _backend::_format_string(stream->suffix);
      }
      else if (stream->prepend_flag) KATAGRAFEAS_COLD
      {
        stream->underlying_ostream << _backend::_format_string(stream->prefix);
        stream->underlying_ostream << _backend::_format_string(prefix);
        stream->prepend_flag = false;
      }

      return stream->buffer->sputc(static_cast<char>(character));
    }
    
    int _interceptor::sync()
    {
      stream->prepend_flag = true;
      return 0;
    }
  }

}
#endif