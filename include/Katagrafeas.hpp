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
#include <ostream>   // for std::ostream
#include <streambuf> // for std::streambuf
#include <iostream>  // for std::cerr
#include <vector>    // for std::vector
#include <cstddef>   // for size_t
#include <string>    // for std::string
//---Katagrafeas library---------------------------------------------------------------------------
namespace Katagrafeas
{
//---Katagrafeas library: frontend forward declarations--------------------------------------------
  inline namespace Frontend
  {
    // library version
    #define KATAGRAFEAS_VERSION       001000000L
    #define KATAGRAFEAS_VERSION_MAJOR 1
    #define KATAGRAFEAS_VERSION_MINOR 0
    #define KATAGRAFEAS_VERSION_PATCH 0

    // allows easy ostream redirection
    class Stream final : public std::streambuf {
      public:
        inline Stream(std::ostream& ostream, std::string prefix = "", std::string postfix = "") noexcept;
        inline ~Stream() noexcept;
        // redirect stream (and backup its original buffer)
        inline void link(std::ostream& ostream) noexcept;
        // restore stream's original buffer
        void restore(std::ostream& ostream) noexcept;
        // output to stream directly
        template<typename T>
        inline std::ostream& operator<<(const T& text) const noexcept;
        // manipulator specialization
        inline std::ostream& operator<<(std::ostream& (*manipulator)(std::ostream&)) const noexcept;
      protected:
        virtual int_type overflow(int_type c) override;
        virtual int sync() override;
      private:
        //
        signed level;
        // ostream with backuped buffer
        struct Backup {
          std::ostream* ostream;
          std::streambuf* buffer;
        };
        // linked ostream backups
        std::vector<Backup> backups_;
        // output buffer
        std::streambuf* buffer_;
        mutable std::ostream ostream_;
        bool prepend_;
        const std::string prefix_;
        const std::string postfix_;
    };
  }
//---Katagrafeas library: frontend definitions-----------------------------------------------------
  inline namespace Frontend
  {
    Stream::Stream(std::ostream& ostream, std::string prefix, std::string postfix) noexcept
      : // member initialization list
      buffer_(ostream.rdbuf()),
      ostream_(this),
      prepend_(true),
      prefix_(prefix),
      postfix_(postfix)
    {}

    Stream::~Stream() noexcept
    {
      for(Backup backup : backups_)
        backup.ostream->rdbuf(backup.buffer);
    }

    void Stream::link(std::ostream& ostream) noexcept
    {
      // backup stream 
      backups_.push_back(Backup{&ostream, ostream.rdbuf()});

      // redirect stream
      ostream.rdbuf(this);
    }

    void Stream::restore(std::ostream& ostream) noexcept
    {
      // traverse backups backwards
      for(size_t i = backups_.size() - 1; i < backups_.size(); --i) {
        // find stream in backups
        if (backups_[i].ostream == &ostream) {
          // restore buffer
          ostream.rdbuf(backups_[i].buffer);

          // remove from backup list
          backups_.erase(backups_.begin() + i);

          // stream was restored, no more work is necessary
          return;
        }
      }

      // if we end up here it's because the ostream is not part of the backup list
      std::cerr << "error: Stream: couldn't restore ostream; ostream not found in backups\n";
    }

    template<typename T>
    std::ostream& Stream::operator<<(const T& text) const noexcept
    {
      // output to stream
      return ostream_ << text;
    }

    std::ostream& Stream::operator<<(std::ostream& (*manipulator)(std::ostream&)) const noexcept
    {
      // apply manipulator
      return manipulator(ostream_);
    }
    
    Stream::int_type Stream::overflow(int_type c)
    {
      if (c != traits_type::eof()) {
        if (prepend_) {
          buffer_->sputn(prefix_.c_str(), prefix_.length());
          prepend_ = false;
        }

        else if (c == '\n') {
          buffer_->sputn(postfix_.c_str(), postfix_.length());
        }

        return buffer_->sputc(c);
      }

      return c;
    }

    int Stream::sync() {
      prepend_ = true;
      return buffer_->pubsync();
    }
  }
}
#endif
