#include <haste/test>
#include <haste/json.hpp>
#include <map>
#include <unordered_map>
#include <vector>
#include <iostream>

using namespace haste;
using namespace haste::mark2;

unittest("is_array_like") {
  static_assert(is_array_like<array<int, 3>>::value);
  static_assert(!is_array_like<vector<int>>::value);
}

unittest("is_continuous") {
  static_assert(is_continuous<vector<int>>::value);
  static_assert(is_continuous<array<int, 3>>::value);
  static_assert(!is_continuous<map<string, int>>::value);
  static_assert(!is_continuous<unordered_map<string, int>>::value);
}

unittest("is_associative") {
  static_assert(is_associative<map<string, int>>::value);
  static_assert(is_associative<unordered_map<string, int>>::value);
}

unittest("Convert empty associative array.") {
  assert_eq("{}", to_json(map<string, int> {}));
  assert_eq("{}", to_json(from_json<map<string, int>>("{}")));
}

unittest("Convert associative array with one element.") {
  assert_eq("{\"one\":1}", to_json(map<string, int> { { "one", 1 } }));
  assert_eq("{\"one\":1}", to_json(from_json<map<string, int>>("{\"one\":1}")));
}

unittest("Convert associative array with multiple elements.") {
  assert_eq(
    "{\"one\":1,\"three\":3,\"two\":2}",
    to_json(map<string, int> { { "one", 1 }, { "two", 2 }, { "three", 3 } }));

  assert_eq(
    "{\"one\":1,\"three\":3,\"two\":2}",
    to_json(from_json<map<string, int>>("{\"one\":1,\"three\":3,\"two\":2}")));
}

unittest("Convert empty unordered map.") {
  assert_eq("{}", to_json(unordered_map<string, int> {}));
  assert_eq("{}", to_json(from_json<unordered_map<string, int>>("{}")));
}

unittest("Convert unordered map with one element.") {
  assert_eq("{\"one\":1}", to_json(unordered_map<string, int> { { "one", 1 } }));
  assert_eq("{\"one\":1}", to_json(from_json<unordered_map<string, int>>("{\"one\":1}")));
}


