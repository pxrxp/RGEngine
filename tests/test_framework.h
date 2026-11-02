#pragma once

#include <cmath>
#include <functional>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

// Minimal test harness: no external dependency, registered tests run in
// declaration order and the process exits non-zero when anything fails.
namespace test {

struct Case
{
  std::string name;
  std::function<void()> fn;
};

inline std::vector<Case>&
registry()
{
  static std::vector<Case> cases;
  return cases;
}

inline int&
failure_count()
{
  static int failures = 0;
  return failures;
}

struct Registrar
{
  Registrar(const char* name, std::function<void()> fn)
  {
    registry().push_back(Case{ name, std::move(fn) });
  }
};

inline void
fail(const char* file, int line, const std::string& message)
{
  ++failure_count();
  std::cerr << file << ':' << line << ": " << message << '\n';
}

} // namespace test

#define TEST_CASE(name)                                                       \
  static void name();                                                         \
  static const ::test::Registrar registrar_##name(#name, name);               \
  static void name()

#define CHECK(condition)                                                      \
  do {                                                                        \
    if (!(condition))                                                         \
      ::test::fail(__FILE__, __LINE__, "CHECK failed: " #condition);          \
  } while (false)

#define CHECK_NEAR(actual, expected, tolerance)                               \
  do {                                                                        \
    const double check_actual = static_cast<double>(actual);                  \
    const double check_expected = static_cast<double>(expected);              \
    const double check_tolerance = static_cast<double>(tolerance);            \
    if (!(std::abs(check_actual - check_expected) <= check_tolerance)) {      \
      std::ostringstream check_message;                                       \
      check_message << "CHECK_NEAR failed: " #actual " = " << check_actual    \
                    << ", expected " << check_expected << " +/- "             \
                    << check_tolerance;                                       \
      ::test::fail(__FILE__, __LINE__, check_message.str());                  \
    }                                                                         \
  } while (false)
