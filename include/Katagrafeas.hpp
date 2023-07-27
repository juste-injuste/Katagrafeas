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

    //
    enum class Pattern {
      YY,
      MM,
      DD,
      hh,
      mm,
      ss
    };
    
    std::function<std::string(void)> parse_datetime(std::string string) noexcept;

    size_t find_pattern_position(const std::string& string, const std::string& pattern) noexcept;
  }
// --Katagrafeas library: frontend struct and class definitions-------------------------------------
  inline namespace Frontend
  {
    class Stream final : public std::streambuf {
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
      protected:
        // overloaded std::streambuf methods to allow prefixing and suffixing
        inline virtual int_type overflow(int_type c) override;
        inline virtual int sync() override;
      private:
        // store intercepted ostreams
        std::vector<std::unique_ptr<Backend::Interceptor>> backups_;
        // output streambuf
        std::streambuf* buffer_;
        // underlying ostream linked to the output buffer
        std::ostream ostream_;
        // prefix for new messages
        const std::string prefix_;
        // suffix for newlines
        const std::string suffix_;
        // prepending flag
        bool prepend_;
      friend class Backend::Interceptor;
    };
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
// --Katagrafeas library: frontend struct and class member definitions------------------------------
  inline namespace Frontend
  {
    Stream::Stream(std::ostream& ostream, const char* prefix, const char* suffix) noexcept
      : // member initialization list
      buffer_(ostream.rdbuf()),
      ostream_(this),
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
      return ostream_ << text;
    }

    std::ostream& Stream::operator<<(std::ostream& (*manipulator)(std::ostream&)) noexcept
    {
      // apply manipulator
      return manipulator(ostream_);
    }
    
    Stream::int_type Stream::overflow(int_type c)
    {
      if (c == traits_type::eof())
        return c;

      if (prepend_) {
        buffer_->sputn(prefix_.c_str(), prefix_.length());
        prepend_ = false;
      }

      else if (c == '\n') {
        buffer_->sputn(suffix_.c_str(), suffix_.length());
      }

      return buffer_->sputc(c);
    }

    int Stream::sync() {
      // buffer will get flushed, 
      prepend_ = true;
      return buffer_->pubsync();
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
      if (c == traits_type::eof())
        return c;

      if (stream->prepend_) {
        stream->buffer_->sputn(stream->prefix_.c_str(), stream->prefix_.length());
        stream->buffer_->sputn(prefix_.c_str(), prefix_.length());
        stream->prepend_ = false;
      }

      else if (c == '\n') {
        stream->buffer_->sputn(suffix_.c_str(), suffix_.length());
        stream->buffer_->sputn(stream->suffix_.c_str(), stream->suffix_.length());
      }

      return stream->buffer_->sputc(c);
    }

    int Interceptor::sync()
    {
      return stream->sync();
    }
std::function<std::string(void)> get_datatime_parser(std::string string) noexcept {
    std::vector<std::pair<Pattern, size_t>> replacement_list;

    size_t YY_pos = find_pattern_position(string, "YY");
    if (YY_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::YY, YY_pos);

    size_t MM_pos = find_pattern_position(string, "MM");
    if (MM_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::MM, MM_pos);

    size_t DD_pos = find_pattern_position(string, "DD");
    if (DD_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::DD, DD_pos);

    size_t hh_pos = find_pattern_position(string, "hh");
    if (hh_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::hh, hh_pos);

    size_t mm_pos = find_pattern_position(string, "mm");
    if (mm_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::mm, mm_pos);

    size_t ss_pos = find_pattern_position(string, "ss");
    if (ss_pos != std::string::npos)
        replacement_list.emplace_back(Pattern::ss, ss_pos);

    return [=]() -> std::string {
        std::string new_string = string;
        auto now = std::chrono::system_clock::now();
        std::time_t time = std::chrono::system_clock::to_time_t(now);
        std::tm* time_info = std::localtime(&time);
        
        for (const auto& pattern : replacement_list) {
            switch (pattern.first) {
                case Pattern::YY:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_year + 1900).substr(2, 2));
                    continue;
                case Pattern::MM:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_mon + 1001).substr(2, 2));
                    continue;
                case Pattern::DD:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_mday + 1000).substr(2, 2));
                    continue;
                case Pattern::hh:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_hour + 1000).substr(2, 2));
                    continue;
                case Pattern::mm:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_min + 1000).substr(2, 2));
                    continue;
                case Pattern::ss:
                    new_string.replace(pattern.second, 2, std::to_string(time_info->tm_sec + 1000).substr(2, 2));
                    continue;
            }
        }
        return new_string;
    };
}

    size_t find_pattern_position(const std::string& string, const std::string& pattern) noexcept
    {
      // find position of first occurence
      size_t pos = string.find(pattern);

      // validate it is the only occurence
      return (pos == string.rfind(pattern)) ? pos : std::string::npos;
    }
  }
}
#endif