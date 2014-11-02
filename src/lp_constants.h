#ifndef __LP_CONSTANTS_H__
#define __LP_CONSTANTS_H__

// platform flags & macros {{{
#ifdef _DEBUG
#  define DEBUG 1
#endif
#if defined _WIN32 || defined WIN32
#  define LP_OS_WIN 1
#  define WIN32 1
#  if defined _WIN64 || defined WIN64
#    define WIN64 1
#    define LP_OS_WIN64 1
#    define LP_OS_STRING "win_64"
#    define LP_64BIT 1
#  else
#    define LP_OS_WIN32 1
#    define LP_OS_STRING "win_32"
#  endif
#  define UNICODE 1
#  define _UNICODE 1
#  define _WIN32_DCOM 1
#  define sleep(n) Sleep(1000 * n)
#  ifndef __MINGW32__
#    define snprintf    _snprintf
#  endif
#endif
#if defined linux || defined __linux__
#  define LP_OS_LINUX 1 
#  define LP_OS_STRING "linux"
#endif
// }}}

#include <string>
#include <memory>
#include <unordered_map>
#include <vector>

#include <lua.hpp>
extern "C" {
#include <lfs.h>
}

// constants {{{
#define LP_VERSION "0.9.0"
#ifdef LP_PUBLIC
#      define LP_EXPORT
#else
#      define LP_EXPORT extern
#endif
#ifdef LP_OS_WIN
#  define LP_MAX_PATH_BYTE (MAX_PATH*5+1)
#  define LP_MAX_PATH MAX_PATH+3
#else
#  define LP_MAX_PATH_BYTE (PATH_MAX*5+1)
#  define LP_MAX_PATH PATH_MAX+3
#endif
#define LP_LOCALE "ja_JP.UTF-8"
// }}}

#define LP_OSSTR(v) (lp::unique_oschar_ptr(lp::platform::utf82oschar(v)).get())
#define LP_OSSTRV(name, v)  lp::unique_oschar_ptr name = (lp::unique_oschar_ptr(lp::platform::utf82oschar(v)));

// typedef {{{
namespace lp {
  typedef std::unique_ptr<std::string> unique_string_ptr;
  typedef std::unique_ptr<wchar_t[]> unique_wchar_ptr;
  typedef std::unique_ptr<char[]> unique_char_ptr;
#ifdef LP_OS_WIN
  typedef wchar_t oschar;
  typedef std::wstring osstring;
#else
  typedef char oschar;
  typedef std::string osstring;
#endif
  typedef std::unique_ptr<oschar[]> unique_oschar_ptr;

  typedef void (*menu_func)(void *data);
  typedef void (*scoped_func)();

  const int MESSAGE_INFO  = 0;
  const int MESSAGE_WARN  = 1;
  const int MESSAGE_ERROR = 2;
  const int MESSAGE_ASK   = 3;

  const int MESSAGE_NO = 0;
  const int MESSAGE_OK = 1;

  const int SHOW_CENTER = 0;
  const int SHOW_COORD  = 1;
  const int SHOW_MOUSE  = 2;
  const int SHOW_WINDOW = 3;

}
// }}}

#endif
