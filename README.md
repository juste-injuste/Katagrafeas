# Katagrafeas (C++11 library)

[![GitHub releases](https://img.shields.io/github/v/release/juste-injuste/Katagrafeas.svg)](https://github.com/juste-injuste/Katagrafeas/releases)
[![License](https://img.shields.io/github/license/juste-injuste/Katagrafeas.svg)](LICENSE)

Katagrafeas is a simple and lightweight C++11 (and newer) library that allows you easily redirect streams towards a common destination.

---

## Usage

Katagrafeas offers

---

### Stream

```
Stream(buffer)
```
description

_Constructor_:
* `buffer` sets the destination buffer. The default is `std::cout.rdbuf()`.

_Methods_:
* `link(ostream)` backup and redirect ostream;
* `restore(ostream)` restore ostream's original buffer.
* `restore_all()` restore all ostreams original buffer.

_Overloads_:
* `operator <<(text)`
* `operator <<(manipulator)`

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

The current library version is given via `Katagrafeas::Version::NUMBER`.

---

## Details

---

## Disclosure

---

## History

Version 0.1.0 - xxx

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