#pragma once

#include <iostream>

// Console logging is a development aid. In release builds (NDEBUG) these
// streams go nowhere, so a shipped executable prints nothing. Fatal startup
// errors in App::init() deliberately still write to std::cerr.
namespace Util {

inline std::ostream&
log_out()
{
#ifdef NDEBUG
  static std::ostream null_stream(nullptr);
  return null_stream;
#else
  return std::cout;
#endif
}

inline std::ostream&
log_err()
{
#ifdef NDEBUG
  static std::ostream null_stream(nullptr);
  return null_stream;
#else
  return std::cerr;
#endif
}

} // namespace Util
