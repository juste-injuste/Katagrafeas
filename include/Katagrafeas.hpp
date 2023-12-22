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
#if defined(__STDCPP_THREADS__) and not defined(KATAGRAFEAS_NOT_THREADSAFE)
# define KATAGRAFEAS_THREADSAFE
# include <mutex>
#endif
#include <cstdio>    // for std::sprintf
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

# define KATAGRAFEAS_LOG(...)             // log a formattable message
# define KATAGRAFEAS_ILOG(...)            // log a formattable message using stack-based indentation
# define KATAGRAFEAS_WARNING(...)         // issue a warning
# define KATAGRAFEAS_ERROR(message, code) // issue an error along with a code

#if not defined(KATAGRAFEAS_MAX_LEN)
# define KATAGRAFEAS_MAX_LEN 256
#endif

  namespace Global
  {
    std::ostream log{std::clog.rdbuf()};
    std::ostream wrn{std::clog.rdbuf()};
    std::ostream err{std::cerr.rdbuf()};
  }
//---Katagrafeas library: backend forward declarations------------------------------------------------------------------
  namespace _backend
  {
# if defined(__GNUC__) and (__GNUC__ >= 9)
#   define KATAGRAFEAS_COLD [[unlikely]]
#   define KATAGRAFEAS_HOT  [[likely]]
# elif defined(__clang__) and (__clang_major__ >= 12)
#   define KATAGRAFEAS_COLD [[unlikely]]
#   define KATAGRAFEAS_HOT  [[likely]]
# else
#   define KATAGRAFEAS_COLD
#   define KATAGRAFEAS_HOT
# endif

# if defined (KATAGRAFEAS_THREADSAFE)
    thread_local char _log_buffer[KATAGRAFEAS_MAX_LEN];
    thread_local char _ilog_buffer[KATAGRAFEAS_MAX_LEN];
    thread_local char _wrn_buffer[KATAGRAFEAS_MAX_LEN];
    std::mutex _log_mtx;
    std::mutex _ilog_mtx;
    std::mutex _wrn_mtx;
#   define KATAGRAFEAS_LOG_LOCK  std::lock_guard<std::mutex> _log_lock{_backend::_log_mtx}
#   define KATAGRAFEAS_ILOG_LOCK std::lock_guard<std::mutex> _ilog_lock{_backend::_ilog_mtx}
#   define KATAGRAFEAS_WRN_LOCK  std::lock_guard<std::mutex> _wrn_lock{_backend::_wrn_mtx}
# else
    char _log_buffer[KATAGRAFEAS_MAX_LEN];
    char _ilog_buffer[KATAGRAFEAS_MAX_LEN];
    char _wrn_buffer[KATAGRAFEAS_MAX_LEN];
#   define KATAGRAFEAS_LOG_LOCK
#   define KATAGRAFEAS_ILOG_LOCK
#   define KATAGRAFEAS_WRN_LOCK
# endif

    // intercept individual ostreams to use a unique prefix and suffix
    class _interceptor final : public std::streambuf
    {
    public:
      _interceptor(Stream* stream, std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
        _ostream{&ostream},
        _stream{stream},
        _prefix{prefix},
        _suffix{suffix}
      {}
      
      ~_interceptor() noexcept
      {
        _ostream->rdbuf(_buffer_backup);
      }

      std::ostream*   const _ostream;
    private:
      std::streambuf* const _buffer_backup = _ostream->rdbuf();
      Stream* const         _stream;
      const char* const     _prefix;
      const char* const     _suffix;

      inline
      int_type overflow(int_type character) override;
      
      inline
      int sync() override;
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

        for (unsigned k = indentation; k; --k)
        {
          Global::log << ' ';
        }

        Global::log << text << std::endl;

        indentation += 2;
      }

      ~_indentedlog() noexcept
      {
        indentation -= 2;
      }
    private:
      static unsigned indentation;
    };

    unsigned _indentedlog::indentation = 0;
    
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
    inline
    Stream(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    
    inline // restore all ostreams orginal buffer
    ~Stream() noexcept;

    inline // redirect ostream (and backup its original buffer)
    void link(std::ostream& ostream, const char* prefix = "", const char* suffix = "") noexcept;
    
    inline // restore ostream's original buffer
    bool restore(std::ostream& ostream) noexcept;
    
    inline // restore all ostreams orginal buffer
    void restore_all() noexcept;

    template<typename T> // interact with general ostream
    inline Stream& operator<<(const T& anything) noexcept;
    inline Stream& operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept;
  private:
    std::vector<std::unique_ptr<_backend::_interceptor>> _backups;
    std::streambuf* _buffer;                   // output buffer
    std::ostream _underlying_ostream{_buffer}; // underlying ostream linked to the output buffer
    std::ostream _general_ostream{nullptr};    // ostream for general io
    const char* const _prefix;                 // prefix for new messages
    const char* const _suffix;                 // suffix for newlines
    bool _prepend_flag = true;                 // prepending flag
  friend class _backend::_interceptor;
  };

# undef  KATAGRAFEAS_LOG
# define KATAGRAFEAS_LOG(...)                                              \
    [&](const char* caller){                                               \
      sprintf(_backend::_log_buffer, __VA_ARGS__);                         \
      KATAGRAFEAS_LOG_LOCK;                                                \
      Global::log << caller << ": " << _backend::_log_buffer << std::endl; \
    }(__func__)

# undef  KATAGRAFEAS_ILOG
# define KATAGRAFEAS_ILOG(...)            KATAGRAFEAS_ILOG_PROX(__LINE__, __VA_ARGS__)
# define KATAGRAFEAS_ILOG_PROX(line, ...) KATAGRAFEAS_ILOG_IMPL(line,     __VA_ARGS__)
# define KATAGRAFEAS_ILOG_IMPL(line, ...)             \
    Katagrafeas::_backend::_indentedlog ilog_##line { \
      [&]{                                            \
        sprintf(_backend::_ilog_buffer, __VA_ARGS__); \
        return buffer;                                \
      }(), __func__}

# undef  KATAGRAFEAS_WARNING
# define KATAGRAFEAS_WARNING(...)                                                         \
    [&](const char* caller){                                                              \
      sprintf(_backend::_wrn_buffer, __VA_ARGS__);                                        \
      KATAGRAFEAS_WRN_LOCK;                                                               \
      Global::wrn << "warning: " << caller << ": " << _backend::_wrn_buffer << std::endl; \
    }(__func__)

# undef  KATAGRAFEAS_ERROR
# define KATAGRAFEAS_ERROR(message, code)
//----------------------------------------------------------------------------------------------------------------------
  Stream::Stream(std::ostream& ostream, const char* prefix, const char* suffix) noexcept :
    _buffer{ostream.rdbuf()},
    _prefix{prefix},
    _suffix{suffix}
  {
    KATAGRAFEAS_LOG("what the fuck 1");
    link(_general_ostream);
  }

  Stream::~Stream() noexcept
  {

    restore_all();
  }

  void Stream::link(std::ostream& ostream, const char* prefix, const char* suffix) noexcept
  {
    _backups.emplace_back(new _backend::_interceptor(this, ostream, prefix, suffix));
    ostream.rdbuf(_backups.back().get()); // redirect towards the new interceptor
  }

  bool Stream::restore(std::ostream& ostream) noexcept
  {
    for (size_t k = _backups.size() - 1; k; --k)
    {
      if (_backups[k]->_ostream == &ostream)
      {
        _backups.erase(_backups.begin() + k);
        return true;
      }
    }

    Global::wrn << "warning: Stream::restore(std::ostream&): ostream was not in in backup list" << std::endl;
    return false;
  }

  void Stream::restore_all() noexcept
  {
    _backups.clear();
  }

  template<typename T>
  Stream& Stream::operator<<(const T& anything) noexcept
  {
    _general_ostream << anything;
    return *this;
  }

  Stream& Stream::operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept
  {
    manipulator(_general_ostream);
    return *this;
  }
// --Katagrafeas library: frontend struct and class member definitions--------------------------------------------------

  namespace _backend
  {
    _interceptor::int_type _interceptor::overflow(int_type character)
    {
      if (character == '\n') KATAGRAFEAS_COLD
      {
        _stream->_underlying_ostream << _backend::_format_string(_suffix);
        _stream->_underlying_ostream << _backend::_format_string(_stream->_suffix);
      }
      else if (_stream->_prepend_flag) KATAGRAFEAS_COLD
      {
        _stream->_underlying_ostream << _backend::_format_string(_stream->_prefix);
        _stream->_underlying_ostream << _backend::_format_string(_prefix);
        _stream->_prepend_flag = false;
      }

      return _stream->_buffer->sputc(static_cast<char>(character));
    }
    
    int _interceptor::sync()
    {
      _stream->_prepend_flag = true;
      return 0;
    }
  }

}
#endif