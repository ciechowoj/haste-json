#include <haste/test>
#include <haste/json.hpp>
#include <array>
#include <iostream>

using namespace haste;
using namespace haste::mark2;

unittest("Convert floating point values.") {
  assert_eq(123.456, from_json<double>(to_json(123.456)));
  assert_eq(1.0, from_json<double>(to_json(1.0)));
}


unittest("Convert array of floating point values.") {
  from_json<std::array<double, 2>>("[1,2]");
  assert_eq("[1,2]", to_json(from_json<std::array<double, 2>>("[1,2]")));
}

