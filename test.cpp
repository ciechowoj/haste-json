#include <haste/test>
#include <haste/json-mk2>
#include <iostream>

using namespace haste;
using namespace haste::mark2;

int main() {
  return haste::run_all_tests() ? 0 : 1;
}

template <class T>
void assert_json_fro_and_to(const string& x, call_site_t site = {}) {
  auto y = to_json(from_json<T>(x));
  assert_eq(y, x, site);
}

struct dummy_nested_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
};

struct dummy_struct_with_string {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", std::string) second_field;
};

struct dummy_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
  JSON_PROPERTY("third_field", dummy_nested_struct) third_field;
};

struct test_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
  JSON_PROPERTY("first_field", int) third_field;
};

struct test_struct2 {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
  JSON_PROPERTY("third_field", test_struct) third_field;
};

struct test_struct_no_json {
  int first_field;
};

unittest("Test has_json_property_type_trait.") {
  static_assert(has_json_property_at_n<0, test_struct>::value);
  static_assert(has_json_property<test_struct>);
  static_assert(!has_json_property<test_struct_no_json>);
  // static_assert(has_json_property<dummy_struct_with_string>);
  // static_assert(has_json_property<std::vector<int>>);

  static_assert(std::is_standard_layout<std::vector<int>>::value);
  std::cout << sizeof(std::vector<int>) * 8 << "\n";
}

unittest("Serialize simple structure.") {
  dummy_nested_struct x;
  x.first_field = 1;
  x.second_field = 2;

  assert_eq("{\"first_field\":1,\"second_field\":2}", to_json(x));
}

unittest("Serialize nested structure.") {
  dummy_struct x;
  x.first_field = 1;
  x.second_field = 2;
  x.third_field.first_field = 3;
  x.third_field.second_field = 4;

  assert_eq("{\"first_field\":1,\"second_field\":2,\"third_field\":{\"first_field\":3,\"second_field\":4}}", to_json(x));
}

unittest("De-serialize simple structure.") {
  auto x = from_json<dummy_nested_struct>("{\"first_field\":1,\"second_field\":2}");
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
}

unittest("De-serialize simple structure - reject json with missing comma.") {
  assert_throws([] { from_json<dummy_nested_struct>("{\"first_field\":1\"second_field\":2}"); });
}

unittest("De-serialize nested structure.") {
  auto x = from_json<dummy_struct>("{\"first_field\":1,\"second_field\":2,\"third_field\":{\"first_field\":3,\"second_field\":4}}");
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
  assert_eq(3, x.third_field.first_field);
  assert_eq(4, x.third_field.second_field);
}

unittest("De-serialize nested structure - white-spaces.") {
  string json = R"(
    {
      "first_field": 1,
      "second_field": 2,
      "third_field":
      {
        "first_field": 3,
        "second_field": 4
      }
    }
  )";

  auto x = from_json<dummy_struct>(json);
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
  assert_eq(3, x.third_field.first_field);
  assert_eq(4, x.third_field.second_field);
}

struct long_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
  JSON_PROPERTY("third_field", int) third_field;
  JSON_PROPERTY("fourth_field", int) fourth_field;
  JSON_PROPERTY("fifth_field", int) fifth_field;
  JSON_PROPERTY("sixth_field", int) sixth_field;
  JSON_PROPERTY("seventh_field", int) seventh_field;
};

unittest("De-serialize nested structure - members are out of order.") {
  string json = R"(
    {
      "sixth_field": 6,
      "second_field": 2,
      "seventh_field": 7,
      "third_field": 3,
      "fifth_field": 5,
      "first_field": 1,
      "fourth_field": 4,
    }
  )";

  auto x = from_json<long_struct>(json);
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
  assert_eq(3, x.third_field);
  assert_eq(4, x.fourth_field);
  assert_eq(5, x.fifth_field);
  assert_eq(6, x.sixth_field);
  assert_eq(7, x.seventh_field);
}

unittest("De-serialize nested structure - member should be ignored.") {
  string json = R"(
    {
      "sixth_field": 6,
      "second_field": 2,
      "seventh_field": 7,
      "third_field": 3,
      "fifth_field": 5,
      "first_field": 1,
      "fourth_field": 4,
      "eight_field": 8,
    }
  )";

  auto x = from_json<long_struct>(json);
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
  assert_eq(3, x.third_field);
  assert_eq(4, x.fourth_field);
  assert_eq(5, x.fifth_field);
  assert_eq(6, x.sixth_field);
  assert_eq(7, x.seventh_field);
}

unittest("De-serialize an array.") {
  string json = R"(
    {
      "sixth_field": 6,
      "second_field": 2,
      "seventh_field": 7,
      "third_field": 3,
      "fifth_field": 5,
      "first_field": 1,
      "fourth_field": 4,
      "eight_field": 8,
    }
  )";

  auto x = from_json<long_struct>(json);
  assert_eq(1, x.first_field);
  assert_eq(2, x.second_field);
  assert_eq(3, x.third_field);
  assert_eq(4, x.fourth_field);
  assert_eq(5, x.fifth_field);
  assert_eq(6, x.sixth_field);
  assert_eq(7, x.seventh_field);
}

unittest("Integer serialization.") {
  assert_json_fro_and_to<int>("-123456789");
}

unittest("Array serialization.") {
  assert_json_fro_and_to<vector<int>>("[1,2,3,4,5]");
}




