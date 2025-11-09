#include <fstream>
#include <iostream>


namespace _impl
{
  class _interceptor : public std::streambuf
  {
  public:
    _interceptor(std::ostream& ostream_, std::ostream* const logger_) noexcept :
      _ostream(ostream_),
      _buffer_backup(_ostream.rdbuf(logger_->rdbuf()))
    {}

    ~_interceptor() noexcept
    {
      _ostream.rdbuf(_buffer_backup);
    }

    inline auto virtual overflow(int_type character) noexcept -> int_type final
    {
      std::cerr << "wtf.";
      return _ostream.rdbuf()->sputc(static_cast<char>(character));
    }

    inline auto virtual underflow() noexcept -> int_type final
    {
      return EOF;
    }

    inline auto virtual sync() noexcept -> int final
    {
      return pubsync();
    }

  private:
    std::ostream&         _ostream;
    std::streambuf* const _buffer_backup;
  };
}

class Logger : public std::ostream
{
public:
  Logger(std::ostream& destination_) noexcept :
    std::ostream(link(destination_)),
    _destination(destination_.rdbuf())
  {}

  auto link(std::ostream& stream) -> _impl::_interceptor*
  {

    return new _impl::_interceptor(stream, this);
  }

private:
  std::streambuf* const _destination;
};

int main()
{
  std::ofstream log_file("hello-world.log");

  Logger logger(log_file);

  logger << "Hello, world!\n";

  return 0;
}