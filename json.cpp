#include <haste/test>
#include <haste/json>

#include <vector>
#include <iostream>

using namespace std;

namespace haste {

void to_json_impl(string& json, std::initializer_list<json_trait> traits) {
  json += "{";

  auto itr = traits.begin();
  auto end = traits.end();

  for (; itr != end; ++itr) {
    auto&& trait = *itr;

    json += "\"";
    json += trait.name;
    json += "\":";

    switch (trait.code) {
      case 0: trait.to(json, trait.data); break;
      case 1: json += std::to_string(*((int*)trait.data)); break;
    }

    if (itr + 1 != end) {
      json += ",";
    }
  }

  json += "}";
}

struct dummy_nested_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
};

struct dummy_struct {
  JSON_PROPERTY("first_field", int) first_field;
  JSON_PROPERTY("second_field", int) second_field;
  JSON_PROPERTY("third_field", dummy_nested_struct) third_field;
};

unittest() {
  dummy_nested_struct x;
  x.first_field = 1;
  x.second_field = 2;

  assert_eq("{\"first_field\":1,\"second_field\":2}", to_json(x));
}

unittest() {
  dummy_struct x;
  x.first_field = 1;
  x.second_field = 2;
  x.third_field.first_field = 3;
  x.third_field.second_field = 4;

  assert_eq("{\"first_field\":1,\"second_field\":2,\"third_field\":{\"first_field\":3,\"second_field\":4}}", to_json(x));
}

unittest() {
  // vector<int> x = { 1, 2, 3, 4, 5 };
  // assert_eq("[1,2,3,4,5]", serialize(x));
}


}
