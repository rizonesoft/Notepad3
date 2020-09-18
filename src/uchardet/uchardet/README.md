# uchardet

Forked from [freedesktop/uchardet](https://github.com/freedesktop/uchardet)

[uchardet](https://www.freedesktop.org/wiki/Software/uchardet/) is an encoding detector library, which takes a sequence of bytes in an unknown character encoding without any additional information, and attempts to determine the encoding of the text. Returned encoding names are [iconv](https://www.gnu.org/software/libiconv/)-compatible.

uchardet started as a C language binding of the original C++ implementation of the universal charset detection library by Mozilla. It can now detect more charsets, and more reliably than the original implementation.

The original code of universalchardet is available at http://lxr.mozilla.org/seamonkey/source/extensions/universalchardet/

Techniques used by universalchardet are described at http://www.mozilla.org/projects/intl/UniversalCharsetDetection.html

## Supported Languages/Encodings

  * International (Unicode)
    * UTF-8
    * UTF-16BE / UTF-16LE
    * UTF-32BE / UTF-32LE / X-ISO-10646-UCS-4-34121 / X-ISO-10646-UCS-4-21431
  * Arabic
    * ISO-8859-6
    * WINDOWS-1256
  * Bulgarian
    * ISO-8859-5
    * WINDOWS-1251
  * Chinese
    * ISO-2022-CN
    * BIG5
    * EUC-TW
    * GB18030
    * HZ-GB-2312
  * Croatian:
    * ISO-8859-2
    * ISO-8859-13
    * ISO-8859-16
    * Windows-1250
    * IBM852
    * MacCentralEurope
  * Czech
    * Windows-1250
    * ISO-8859-2
    * IBM852
    * MacCentralEurope
  * Danish
    * ISO-8859-1
    * ISO-8859-15
    * WINDOWS-1252
  * English
    * ASCII
  * Esperanto
    * ISO-8859-3
  * Estonian
    * ISO-8859-4
    * ISO-8859-13
    * ISO-8859-13
    * Windows-1252
    * Windows-1257
  * Finnish
    * ISO-8859-1
    * ISO-8859-4
    * ISO-8859-9
    * ISO-8859-13
    * ISO-8859-15
    * WINDOWS-1252
  * French
    * ISO-8859-1
    * ISO-8859-15
    * WINDOWS-1252
  * German
    * ISO-8859-1
    * WINDOWS-1252
  * Greek
    * ISO-8859-7
    * WINDOWS-1253
  * Hebrew
    * ISO-8859-8
    * WINDOWS-1255
  * Hungarian:
    * ISO-8859-2
    * WINDOWS-1250
  * Irish Gaelic
    * ISO-8859-1
    * ISO-8859-9
    * ISO-8859-15
    * WINDOWS-1252
  * Italian
    * ISO-8859-1
    * ISO-8859-3
    * ISO-8859-9
    * ISO-8859-15
    * WINDOWS-1252
  * Japanese
    * ISO-2022-JP
    * SHIFT_JIS
    * EUC-JP
  * Korean
    * ISO-2022-KR
    * EUC-KR / UHC
  * Lithuanian
    * ISO-8859-4
    * ISO-8859-10
    * ISO-8859-13
  * Latvian
    * ISO-8859-4
    * ISO-8859-10
    * ISO-8859-13
  * Maltese
    * ISO-8859-3
  * Polish:
    * ISO-8859-2
    * ISO-8859-13
    * ISO-8859-16
    * Windows-1250
    * IBM852
    * MacCentralEurope
  * Portuguese
    * ISO-8859-1
    * ISO-8859-9
    * ISO-8859-15
    * WINDOWS-1252
  * Romanian:
    * ISO-8859-2
    * ISO-8859-16
    * Windows-1250
    * IBM852
  * Russian
    * ISO-8859-5
    * KOI8-R
    * WINDOWS-1251
    * MAC-CYRILLIC
    * IBM866
    * IBM855
  * Slovak
    * Windows-1250
    * ISO-8859-2
    * IBM852
    * MacCentralEurope
  * Slovene
    * ISO-8859-2
    * ISO-8859-16
    * Windows-1250
    * IBM852
    * MacCentralEurope
  * Spanish
    * ISO-8859-1
    * ISO-8859-15
    * WINDOWS-1252
  * Swedish
    * ISO-8859-1
    * ISO-8859-4
    * ISO-8859-9
    * ISO-8859-15
    * WINDOWS-1252
  * Thai
    * TIS-620
    * ISO-8859-11
  * Turkish:
    * ISO-8859-3
    * ISO-8859-9
  * Vietnamese:
    * VISCII
    * Windows-1258
  * Others
    * WINDOWS-1252

## Installation

### Build from source

If you prefer a development version, clone the git repository:

    git clone https://github.com/PyYoshi/uchardet.git

The source can be browsed at: https://github.com/PyYoshi/uchardet

    mkdir build/ && cd build/
    cmake ..
    make
    make install

## Usage

### Command Line

```
uchardet Command Line Tool
Version 0.0.6

Authors: BYVoid, Jehan
Bug Report: https://bugs.freedesktop.org/enter_bug.cgi?product=uchardet

Usage:
 uchardet [Options] [File]...

Options:
 -v, --version         Print version and build information.
 -h, --help            Print this help.
 ```
### Library

See [uchardet.h](https://github.com/PyYoshi/uchardet/blob/cchardet/src/uchardet.h)

## Licenses

* [Mozilla Public License Version 1.1](http://www.mozilla.org/MPL/1.1/)
* [GNU General Public License, version 2.0](http://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html) or later.
* [GNU Lesser General Public License, version 2.1](http://www.gnu.org/licenses/old-licenses/lgpl-2.1.en.html) or later.

See the file `COPYING` for the complete text of these 3 licenses.
