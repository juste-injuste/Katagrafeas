#include <streambuf>
#include <iostream>
#include <ctime>
#include <cassert>

template< class Inserter >
class FilteringOutputStreambuf : public std::streambuf
{
public:
  FilteringOutputStreambuf(
    std::streambuf* dest,
    Inserter        i,
    bool            deleteWhenFinished = false
  ) noexcept :
    myDest(dest),
    myInserter(i),
    myDeleteWhenFinished(deleteWhenFinished)
  {}
  
  virtual
  ~FilteringOutputStreambuf() noexcept = default;

  virtual
  auto overflow(const int_type character) noexcept -> int override
  {
    int result = EOF ;
    if (character == EOF)
    {
      result = sync();
    }
    else if (myDest != nullptr)
    {
      // assert(character >= 0 && character <= UCHAR_MAX);
      result = myInserter(*myDest, static_cast<char>(character));
    }
    
    return result ;
  }

  virtual
  auto underflow() noexcept -> int override
  {
    return EOF;
  }

  virtual int sync() noexcept override
  {
    return myDest->pubsync();
  }

  virtual std::streambuf* setbuf(char* p , int len) noexcept
  {
    return myDest->pubsetbuf(p, len);
  }

private:
  std::streambuf* myDest;
  Inserter        myInserter;
  bool            myDeleteWhenFinished;
} ;



class TimeStampInserter
{
public:
    TimeStampInserter() noexcept = default;

    int operator()(std::streambuf& dst, const char character)
    {
      static char buffer[128];
      if (myAtStartOfLine && character != '\n')
      {
        auto t      = std::time(nullptr);
        auto time   = std::localtime(&t) ;
        int  length = static_cast<int>(std::strftime(buffer, sizeof( buffer ), "%c: ", time));

        // assert(length > 0);

        if (dst.sputn(buffer, length) != length)
        {
          return EOF;
        }
      }

      myAtStartOfLine = (character == '\n');

      return dst.sputc(character);
    }

private:
  bool myAtStartOfLine = true;
};



int main()
{
  TimeStampInserter ins;
  FilteringOutputStreambuf<TimeStampInserter> test(std::cout.rdbuf(), ins);
  
  std::cout << "test A\n";
  std::cout.rdbuf(&test);
  std::cout << "test B\n";

  return 0;
}