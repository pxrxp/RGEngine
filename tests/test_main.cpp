#include "test_framework.h"

int
main()
{
  int failed_cases = 0;
  for (const auto& test_case : test::registry()) {
    const int before = test::failure_count();
    std::cout << "[ RUN      ] " << test_case.name << '\n';
    test_case.fn();
    if (test::failure_count() == before) {
      std::cout << "[       OK ] " << test_case.name << '\n';
    } else {
      std::cout << "[  FAILED  ] " << test_case.name << '\n';
      ++failed_cases;
    }
  }

  std::cout << "\n" << test::registry().size() << " tests, " << failed_cases
            << " failed\n";
  return failed_cases == 0 ? 0 : 1;
}
