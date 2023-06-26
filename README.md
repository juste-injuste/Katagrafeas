# Katagrafeas (C++11 library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Katagrafeas.svg)](https://github.com/juste-injuste/Katagrafeas/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Katagrafeas.svg)](LICENSE)

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect streams towards a common destination.

---

## Usage

Katagrafeas offers two ways to measure elapsed time:
* [execution_time](#execution_time) function
* [CHRONOMETRO_EXECUTION_TIME](#CHRONOMETRO_EXECUTION_TIME) macro

---

### OStream

```
OStream(buffer)
```
The Stopwatch class allows to measure the time it takes to execute code blocks. A stopwatch may be started, paused stopped, restarted or have its unit set. When a stopwatch is stopped, it displays the elapsed time. Pausing or stopping a stopwatch returns the elapsed time as the `C::duration` type. A stopwatch starts measuring time upon creation.

_Constructor_:
* `buffer` sets the destination buffer. The default is `std::cout.rdbuf()`.

_Methods_:
* `link(ostream)` backup and redirect ostream;
* `restore(ostream)` restore ostream's original buffer.

_Overloads_:
* `operator <<(text)`

_Error_:
* `restore()` will issue `"error: Stream: couldn't restore ostream; ostream not found in backups\n"` if the ostream couldn't be restored.

_Examples_:
```cpp
#include "Katagrafeas.hpp"

int main()
{
  using namespace Katagrafeas;

}
```

---

## Examples

---

## Version

The current version defines the following macros:
```cpp
#define KATAGRAFEAS_VERSION       001000000L
#define KATAGRAFEAS_VERSION_MAJOR 1
#define KATAGRAFEAS_VERSION_MINOR 0
#define KATAGRAFEAS_VERSION_PATCH 0
```

---

## Details

---

## Disclosure

---

## History

Version 1.0.0 - initial release

---

## Installation

Katagrafeas is a header-only library. To use it in your C++ project, simply `#include` the [Katagrafeas.hpp](include/Katagrafeas.hpp) header file.

---

## License

Katagrafeas is released under the MIT License. See the [LICENSE](LICENSE) file for more details.

---

## Author

Justin Asselin (juste-injuste)  
email: justin.asselin@usherbrooke.ca  
GitHub: [juste-injuste](https://github.com/juste-injuste)