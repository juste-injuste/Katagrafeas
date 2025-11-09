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


-----description--------------------------------------------------------------------------------------------------------

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect
streams towards a common destination. See the included README.MD file for more information.

-----inclusion guard--------------------------------------------------------------------------------------------------*/
#ifndef _katagrafeas_hpp
#define _katagrafeas_hpp
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
//---conditionally necessary standard libraries-------------------------------------------------------------------------
#if defined(__STDCPP_THREADS__) and not defined(KTZ_NOT_THREADSAFE)
# define _ktz_impl_THREADSAFE
# include <atomic>   // for std::atomic
# include <mutex>    // for std::mutex, std::lock_guard
#endif
//---Katagrafeas library------------------------------------------------------------------------------------------------
namespace ktz
{
  // ostream redirection aswell as prefixing and suffixing
  class Logger;

# define log_message(...)             // log a message
# define indented_log(...)            // log a message using stack-based indentation
# define KTZ_WARNING(...)         // issue a warning
# define KTZ_ERROR(message, code) // issue an error along with a code

#if defined(KTZ_MAX_LEN)
# define _ktz_impl_MAX_LEN KTZ_MAX_LEN
#else
# define _ktz_impl_MAX_LEN 256
#endif

  namespace _io
  {
    static std::ostream log(std::clog.rdbuf());
    static std::ostream wrn(std::clog.rdbuf());
    static std::ostream err(std::cerr.rdbuf());
  }

  namespace _version
  {
    constexpr unsigned long MAJOR  = 000;
    constexpr unsigned long MINOR  = 001;
    constexpr unsigned long PATCH  = 000;
    constexpr unsigned long NUMBER = (MAJOR * 1000 + MINOR) * 1000 + PATCH;
  }
//---Katagrafeas library: backend forward declarations------------------------------------------------------------------
  namespace _impl
  {
    class _interceptor;
  }
// --Katagrafeas library: frontend struct and class definitions---------------------------------------------------------
  class Logger final : std::ostream
  {
  public:
    inline Logger(const std::ostream& ostream) noexcept;

    inline // redirect ostream (and backup its original buffer)
    void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;

  private:
    std::vector<std::unique_ptr<_impl::_interceptor>> _backups;
    std::streambuf* const _buffer;                      // output buffer
    std::ostream          _underlying_ostream{_buffer}; // underlying ostream linked to the output buffer
    friend _impl::_interceptor;
  };
//---Katagrafeas library: backend forward declarations------------------------------------------------------------------
  namespace _impl
  {
#   define _ktz_impl_PRAGMA(PRAGMA) _Pragma(#PRAGMA)
#   define _ktz_impl_CLANG_IGNORE(WARNING, ...)          \
      _ktz_impl_PRAGMA(clang diagnostic push)            \
      _ktz_impl_PRAGMA(clang diagnostic ignored WARNING) \
      __VA_ARGS__                                        \
      _ktz_impl_PRAGMA(clang diagnostic pop)

// support from clang 12.0.0 and GCC 10.1 onward
# if defined(__clang__) and (__clang_major__ >= 12)
# if __cplusplus < 202002L
#   define _ktz_impl_LIKELY   _ktz_impl_CLANG_IGNORE("-Wc++20-extensions", [[likely]])
#   define _ktz_impl_UNLIKELY _ktz_impl_CLANG_IGNORE("-Wc++20-extensions", [[unlikely]])
# else
#   define _ktz_impl_LIKELY   [[likely]]
#   define _ktz_impl_UNLIKELY [[unlikely]]
# endif
# elif defined(__GNUC__) and (__GNUC__ >= 10)
#   define _ktz_impl_LIKELY   [[likely]]
#   define _ktz_impl_UNLIKELY [[unlikely]]
# else
#   define _ktz_impl_LIKELY
#   define _ktz_impl_UNLIKELY
# endif

// support from clang 3.9.0 and GCC 5.1 onward
# if defined(__clang__)
#   define _ktz_impl_MAYBE_UNUSED __attribute__((unused))
# elif defined(__GNUC__)
#   define _ktz_impl_MAYBE_UNUSED __attribute__((unused))
# else
#   define _ktz_impl_MAYBE_UNUSED
# endif

# if defined(_ktz_impl_THREADSAFE)
#   define _ktz_impl_THREADLOCAL         thread_local
#   define _ktz_impl_ATOMIC(TYPE)        std::atomic<TYPE>
#   define _ktz_impl_MAKE_MUTEX(...)     static std::mutex __VA_ARGS__
#   define _ktz_impl_DECLARE_LOCK(MUTEX) std::lock_guard<decltype(MUTEX)> _lock{MUTEX}
# else
#   define _ktz_impl_THREADLOCAL
#   define _ktz_impl_ATOMIC(TYPE)        TYPE
#   define _ktz_impl_MAKE_MUTEX(...)
#   define _ktz_impl_DECLARE_LOCK(MUTEX)
# endif

    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _log_buf[_ktz_impl_MAX_LEN];
    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _ilg_buf[_ktz_impl_MAX_LEN];
    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _wrn_buf[_ktz_impl_MAX_LEN];
    _ktz_impl_MAKE_MUTEX(_log_mtx, _ilg_mtx, _wrn_mtx);

    struct _interceptor final : public std::streambuf
    {
      friend Logger;
      _interceptor(
        Logger* const     logger_, std::ostream&     ostream_,
        const char* const prefix_, const char* const suffix_
      ) noexcept :
        _logger(*logger_), _ostream(ostream_),
        _prefix(prefix_), _suffix(suffix_)
      {
        // ostream_.rdbuf(this);
      }

      virtual
      ~_interceptor() noexcept
      {
        // _ostream.rdbuf(_buffer_backup);
      }
      
      Logger&               _logger;
      std::ostream&         _ostream;
      // // std::streambuf* const _buffer_backup = _ostream.rdbuf();
      const char* const     _prefix;
      const char* const     _suffix;

      inline auto virtual overflow(int_type character) -> int_type final;
      inline auto virtual underflow() -> int_type final;
      inline auto virtual sync() -> int final;
    };

    inline
    auto _format_string(const char* const format_) -> decltype(std::put_time((const std::tm*)0, ""))
    {
      const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format_);
    }

    class _indented_log final
    {
    public:
      _indented_log(const char* const text, const char* const caller = "") noexcept
      {
        {
          _ktz_impl_DECLARE_LOCK(_log_mtx);
          _io::log << "log: " << caller << ": ";

          for (unsigned k = _indentation(); k--;)
          {
            _io::log << ' ';
          }

          _io::log << text << std::endl;
        }

        _indentation() += 2;
      }

      ~_indented_log() noexcept
      {
        _indentation() -= 2;
      }

    private:
      auto _indentation() -> _ktz_impl_ATOMIC(unsigned)&
      {
        static _ktz_impl_ATOMIC(unsigned) indentation = {0};
        return indentation;
      }
    };

    constexpr void _log(...) noexcept {};
  }

# undef  log_message
# define log_message(...)                         \
    _impl::_log(([&](const char* const caller_){  \
      using namespace ktz;                        \
      std::sprintf(_impl::_log_buf, __VA_ARGS__); \
      _ktz_impl_DECLARE_LOCK(_impl::_log_mtx);    \
      _io::log << "log: " << caller_ << ": ";     \
      _io::log << _impl::_log_buf << std::endl;   \
    }(__func__), 0))

# undef  indented_log
# define indented_log(...)              _ktz_impl_ILOG_PRXY(__LINE__,    __VA_ARGS__)
# define _ktz_impl_ILOG_PRXY(LINE, ...) _ktz_impl_ILOG_IMPL(LINE,        __VA_ARGS__)
# define _ktz_impl_ILOG_IMPL(LINE, ...)           \
    _impl::_indented_log _ilg_##LINE([&](){       \
      using namespace ktz;                        \
      std::sprintf(_impl::_ilg_buf, __VA_ARGS__); \
      return _impl::_ilg_buf;                     \
    }(), __func__)

# undef  KTZ_WARNING
# define KTZ_WARNING(...)                                                        \
    [&](const char* const caller){                                               \
      std::sprintf(_impl::_wrn_buf, __VA_ARGS__);                                \
      _ktz_impl_DECLARE_LOCK(_impl::_wrn_mtx);                                   \
      _io::wrn << "warning: " << caller << ": " << _impl::_wrn_buf << std::endl; \
    }(__func__)

# undef  KTZ_ERROR
# define KTZ_ERROR(return_value, ...)                                          \
    return [&](const char* const caller){                                      \
      std::sprintf(_impl::_err_buf, __VA_ARGS__);                              \
      _ktz_impl_DECLARE_LOCK(_impl::_err_mtx);                                 \
      _io::err << "error: " << caller << ": " << _impl::_err_buf << std::endl; \
    }(__func__), return_value
//----------------------------------------------------------------------------------------------------------------------
  Logger::Logger(const std::ostream& ostream_) noexcept :
    _buffer(ostream_.rdbuf())
  {
    // link(_general_ostream);
  }

  void Logger::link(std::ostream& ostream_, const char* const prefix_, const char* const suffix_) noexcept
  {
    const auto backup = new _impl::_interceptor(this, ostream_, prefix_, suffix_);
    _backups.emplace_back(backup);
    // ostream_.rdbuf(_backups.back().get()); // redirect towards the new interceptor
  }



// --Katagrafeas library: frontend struct and class member definitions--------------------------------------------------
  namespace _impl
  {
    auto _interceptor::overflow(const int_type character_) -> int_type
    {
      // if (character_ == '\n') _ktz_impl_UNLIKELY
      // {
      //   _logger._underlying_ostream << _impl::_format_string(_suffix);
      //   _logger._underlying_ostream << _impl::_format_string(_logger._suffix);
      // }
      // else if (_logger._prepend_flag) _ktz_impl_UNLIKELY
      // {
      //   _logger._underlying_ostream << _impl::_format_string(_logger._prefix);
      //   _logger._underlying_ostream << _impl::_format_string(_prefix);
      //   _logger._prepend_flag = false;
      // }

      return _logger._buffer->sputc(static_cast<char>(character_));
      return character_;
    }

    auto _interceptor::underflow() -> int_type
    {
      return EOF;
    }

    auto _interceptor::sync() -> int
    {
      // _logger._prepend_flag = true;
      return 0;
    }
  }
}
# undef _ktz_impl_PRAGMA
# undef _ktz_impl_CLANG_IGNORE
# undef _ktz_impl_LIKELY
# undef _ktz_impl_UNLIKELY
# undef _ktz_impl_THREADSAFE
# undef _ktz_impl_THREADLOCAL
# undef _ktz_impl_MAKE_MUTEX
# undef _ktz_impl_NODISCARD
# undef _ktz_impl_NODISCARD_REASON
#endif
