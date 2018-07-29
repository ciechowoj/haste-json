#include <haste/test>
#include <haste/json.hpp>
#include <iostream>

using namespace haste;
using namespace haste::mark2;

unittest("Convert empty associative array.") {
  assert_eq("{}", to_json(map<string, int> {}));
}

unittest("Convert associative array with one element.") {
  assert_eq("{\"one\":1}", to_json(map<string, int> { { "one", 1 } }));
}

unittest("Convert associative array with multiple elements.") {
  assert_eq(
    "{\"one\":1,\"three\":3,\"two\":2}",
    to_json(map<string, int> { { "one", 1 }, { "two", 2 }, { "three", 3 } }));
}

unittest("Test") {




}
