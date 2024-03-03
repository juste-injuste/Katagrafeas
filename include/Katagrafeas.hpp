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
#if defined(__STDCPP_THREADS__) and not defined(MUD_NOT_THREADSAFE)
# define _ktz_impl_THREADSAFE
# include <atomic>   // for std::atomic
# include <mutex>    // for std::mutex, std::lock_guard
#endif
//---Katagrafeas library------------------------------------------------------------------------------------------------
namespace mud
{
  // ostream redirection aswell as prefixing and suffixing
  class Logger;

# define MUD_LOG(...)             // log a message
# define MUD_ILOG(...)            // log a message using stack-based indentation
# define MUD_WARNING(...)         // issue a warning
# define MUD_ERROR(message, code) // issue an error along with a code

#if defined(MUD_MAX_LEN)
# define _ktz_impl_MAX_LEN MUD_MAX_LEN
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
  namespace _backend
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
#   define _ktz_impl_THREADLOCAL     thread_local
#   define _ktz_impl_ATOMIC(TYPE)    std::atomic<TYPE>
#   define _ktz_impl_MAKE_MUTEX(...) static std::mutex __VA_ARGS__
#   define _ktz_impl_LOCK(MUTEX)     std::lock_guard<decltype(MUTEX)> _lock{MUTEX}
# else
#   define _ktz_impl_THREADLOCAL
#   define _ktz_impl_ATOMIC(TYPE)    TYPE
#   define _ktz_impl_MAKE_MUTEX(...)
#   define _ktz_impl_LOCK(MUTEX)     void(0)
# endif

    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _log_buffer[_ktz_impl_MAX_LEN];
    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _ilog_buffer[_ktz_impl_MAX_LEN];
    _ktz_impl_MAYBE_UNUSED static _ktz_impl_THREADLOCAL char _wrn_buffer[_ktz_impl_MAX_LEN];
    _ktz_impl_MAKE_MUTEX(_log_mtx, _ilog_mtx, _wrn_mtx);

    class _interceptor final : public std::streambuf
    {
    public:
      _interceptor(
        Logger* const     stream, std::ostream&     ostream,
        const char* const prefix, const char* const suffix
      ) noexcept :
        _ostream(&ostream), _stream(stream),
        _prefix(prefix),    _suffix(suffix)
      {}
      
      ~_interceptor() noexcept
      {
        _ostream->rdbuf(_buffer_backup);
      }

      std::ostream* const    _ostream;
    private:
      std::streambuf* const _buffer_backup = _ostream->rdbuf();
      Logger* const         _stream;
      const char* const     _prefix;
      const char* const     _suffix;

      inline auto overflow(int_type character) -> int_type override;
      inline auto sync() -> int override;
    };

    inline
    auto _format_string(const char* const format_) -> decltype(std::put_time((const std::tm*)0, ""))
    {
      const auto time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
      return std::put_time(std::localtime(&time), format_);
    }

    class _indentedlog final
    {
    public:
      _indentedlog(const char* text, const char* caller = "") noexcept
      {
        _io::log << "log: " << caller << ": ";

        for (unsigned k = _indentation(); k--;)
        {
          _io::log << ' ';
        }

        _io::log << text << std::endl;

        _indentation() += 2;
      }

      ~_indentedlog() noexcept
      {
        _indentation() -= 2;
      }
    private:
      _ktz_impl_ATOMIC(unsigned)& _indentation()
      {
        static _ktz_impl_ATOMIC(unsigned) indentation = {0};
        return indentation;
      }
    };
  }
// --Katagrafeas library: frontend struct and class definitions---------------------------------------------------------
  class Logger final
  {
  public:
    inline Logger(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    inline Logger(Logger&& logger) noexcept;
    
    inline // restore all ostreams orginal buffer
    ~Logger() noexcept;

    inline // redirect ostream (and backup its original buffer)
    void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    
    inline // restore ostream's original buffer
    bool restore(std::ostream& ostream) noexcept;
    
    inline // restore all ostreams orginal buffer
    void restore_all() noexcept;

    template<typename T> // interact with general ostream
    inline Logger& operator<<(const T& anything) noexcept;
    inline Logger& operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept;
  private:
    std::vector<std::unique_ptr<_backend::_interceptor>> _backups;
    std::streambuf* const _buffer;                      // output buffer
    std::ostream          _underlying_ostream{_buffer}; // underlying ostream linked to the output buffer
    std::ostream          _general_ostream{nullptr};    // ostream for general io
    const char* const     _prefix;                      // prefix for new messages
    const char* const     _suffix;                      // suffix for newlines
    bool                  _prepend_flag = true;         // prepending flag
    friend _backend::_interceptor;
  };

# undef  MUD_LOG
# define MUD_LOG(...)                                                   \
    [&](const char* const caller){                                      \
      std::sprintf(_backend::_log_buffer, __VA_ARGS__);                 \
      _ktz_impl_LOCK(_backend::_log_mtx);                               \
      _io::log << caller << ": " << _backend::_log_buffer << std::endl; \
    }(__func__)

# undef  MUD_ILOG
# define MUD_ILOG(...)                 _ktz_impl_ILOG_PROX(__LINE__,    __VA_ARGS__)
# define _ktz_impl_ILOG_PROX(line_number, ...) _ktz_impl_ILOG_IMPL(line_number, __VA_ARGS__)
# define _ktz_impl_ILOG_IMPL(line_number, ...)               \
    mud::_backend::_indentedlog ilog_##line_number {         \
      [&]{                                                   \
        std::sprintf(_backend::_ilog_buffer, __VA_ARGS__);   \
        return buffer;                                       \
      }(), __func__}

# undef  MUD_WARNING
# define MUD_WARNING(...)                                                              \
    [&](const char* const caller){                                                     \
      std::sprintf(_backend::_wrn_buffer, __VA_ARGS__);                                \
      _ktz_impl_LOCK(_backend::_wrn_mtx);                                              \
      _io::wrn << "warning: " << caller << ": " << _backend::_wrn_buffer << std::endl; \
    }(__func__)

# undef  MUD_ERROR
# define MUD_ERROR(return_value, ...)
//----------------------------------------------------------------------------------------------------------------------
  Logger::Logger(std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
    _buffer(ostream.rdbuf()),
    _prefix(prefix),
    _suffix(suffix)
  {
    link(_general_ostream);
  }

  Logger::Logger(Logger&& logger_) noexcept :
    _backups(std::move(logger_._backups)),
    _buffer(logger_._buffer),                   
    _underlying_ostream(logger_._underlying_ostream.rdbuf()),
    _general_ostream(logger_._general_ostream.rdbuf()), 
    _prefix(logger_._prefix),            
    _suffix(logger_._suffix),                     
    _prepend_flag(logger_._prepend_flag)
  {}

  Logger::~Logger() noexcept
  {
    restore_all();
  }

  void Logger::link(std::ostream& ostream_, const char* const prefix_, const char* const suffix_) noexcept
  {
    _backups.emplace_back(new _backend::_interceptor(this, ostream_, prefix_, suffix_));
    ostream_.rdbuf(_backups.back().get()); // redirect towards the new interceptor
  }

  bool Logger::restore(std::ostream& ostream_) noexcept
  {
    for (size_t k = _backups.size() - 1; k; --k)
    {
      if (_backups[k]->_ostream == &ostream_)
      {
        _backups.erase(_backups.begin() + k);
        return true;
      }
    }

    _io::wrn << "warning: Stream::restore(std::ostream&): ostream was not in in backup list" << std::endl;
    return false;
  }

  void Logger::restore_all() noexcept
  {
    _backups.clear();
  }

  template<typename T>
  Logger& Logger::operator<<(const T& anything_) noexcept
  {
    _general_ostream << anything_;
    return *this;
  }

  Logger& Logger::operator<<(std::ostream& (*const manipulator_)(std::ostream&)) noexcept
  {
    manipulator_(_general_ostream);
    return *this;
  }
// --Katagrafeas library: frontend struct and class member definitions--------------------------------------------------
  namespace _backend
  {
    auto _interceptor::overflow(const int_type character_) -> int_type
    {
      if (character_ == '\n') _ktz_impl_UNLIKELY
      {
        _stream->_underlying_ostream << _backend::_format_string(_suffix);
        _stream->_underlying_ostream << _backend::_format_string(_stream->_suffix);
      }
      else if (_stream->_prepend_flag) _ktz_impl_UNLIKELY
      {
        _stream->_underlying_ostream << _backend::_format_string(_stream->_prefix);
        _stream->_underlying_ostream << _backend::_format_string(_prefix);
        _stream->_prepend_flag = false;
      }

      return _stream->_buffer->sputc(static_cast<char>(character_));
    }
    
    int _interceptor::sync()
    {
      _stream->_prepend_flag = true;
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
# undef _ktz_impl_LOCK
# undef _ktz_impl_NODISCARD
# undef _ktz_impl_NODISCARD_REASON
#endif
