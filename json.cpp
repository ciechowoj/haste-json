#include <haste/test>
#include <haste/json>

#include <vector>
#include <iostream>
#include <cctype>

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
      case 0: trait.to(json, trait.cdata); break;
      case 1: json += std::to_string(*((int*)trait.cdata)); break;
    }

    if (itr + 1 != end) {
      json += ",";
    }
  }

  json += "}";
}

inline const char* skip_spaces(const char* itr, const char* end) {
  while (itr < end && std::isspace(*itr)) {
    ++itr;
  }

  return itr;
}

inline const char* expect_string(const char* itr, const char* end, const char* chr) {
  while (itr < end && *chr != '\0' && *itr == *chr) {
    ++itr; ++chr;
  }

  if (*chr != '\0') {
    throw std::runtime_error(chr + std::string(" expected!"));
  }

  return itr;
}

inline const char* skip_string(const char* itr, const char* end, const char* str) {
  const char* rollback = itr;

  while (itr < end && *str != '\0' && *itr == *str) {
    ++itr; ++str;
  }

  if (*str != '\0') {
    return rollback;
  }

  return itr;
}

inline const char* skip_string(const char* itr, const char* end) {
  while (itr < end && *itr != '"') {
    ++itr;
  }

  return itr;
}

inline const char* expect_char(const char* itr, const char* end, char chr) {
  if (itr < end && *itr == chr) {
    return itr + 1;
  }

  throw std::runtime_error(chr + std::string(" expected!"));
}

inline const char* skip_char(const char* itr, const char* end, char chr) {
  if (itr < end && *itr == chr) {
    return itr + 1;
  }

  return itr;
}

inline const char* expect_string_literal(const char* itr, const char* end) {
  itr = expect_char(itr, end, '"');

  while (itr < end && *itr != '"') {
    ++itr;
  }

  if (itr != end) {
    return itr + 1;
  }

  throw std::runtime_error("String literal expected!");
}

inline const char* expect_json(const char* itr, const char* end) {
  if (itr < end) {
    if (*itr == '{') {
      ++itr;

      while (itr < end && *itr != '}') {
        itr = skip_spaces(itr, end);
        itr = expect_string_literal(itr, end);
        itr = skip_spaces(itr, end);
        itr = expect_char(itr, end, ':');
        itr = skip_spaces(itr, end);
        itr = expect_json(itr, end);
        itr = skip_spaces(itr, end);

        if (*itr != '}') {
          itr = expect_char(itr, end, ',');
        }
      }

      return expect_char(itr, end, '}');
    }
    else if (*itr == '[') {
      ++itr;

    }
    else if (isdigit(*itr)) {
      ++itr;

      while (itr < end && isdigit(*itr)) {
        ++itr;
      }

      return itr;
    }
    else {
      throw std::runtime_error("Json expected!");
    }
  }

  return itr;
}

inline const char* from_json_impl(const char* itr, const char* end, std::initializer_list<json_trait> traits) {
  itr = skip_spaces(itr, end);
  itr = expect_char(itr, end, '{');

  while (itr < end && *itr != '}') {
    itr = skip_spaces(itr, end);
    itr = expect_char(itr, end, '"');

    auto traitItr = traits.begin();
    auto traitEnd = traits.end();

    for (; traitItr < traitEnd; ++traitItr) {
      auto jtr = skip_string(itr, end, traitItr->name);

      if (itr != jtr) {
        itr = jtr;
        break;
      }
    }

    if (traitItr == traitEnd) {
      itr = expect_string_literal(itr - 1, end);
      itr = skip_spaces(itr, end);
      itr = expect_char(itr, end, ':');
      itr = skip_spaces(itr, end);
      itr = expect_json(itr, end);
    }
    else {
      itr = expect_char(itr, end, '"');
      itr = skip_spaces(itr, end);
      itr = expect_char(itr, end, ':');
      itr = skip_spaces(itr, end);

      if (itr == end) {
        throw std::runtime_error("Json error!");
      }
      else if (*itr == '{') {
        itr = traitItr->from(itr, end, traitItr->data);
      }
      else if (isdigit(*itr)) {
        auto& result = *((int*)traitItr->data);
        result = 0;

        while (itr < end && isdigit(*itr)) {
          result *= 10;
          result += *itr - '0';
          ++itr;
        }
      }
      else {
        throw std::runtime_error("Json error!");
      }
    }

    itr = skip_spaces(itr, end);

    if (*itr != ',') {
      itr = skip_spaces(itr, end);
      break;
    }
    else {
      ++itr;
      itr = skip_spaces(itr, end);
    }
  }

  return expect_char(itr, end, '}');
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

bool validate_json(const string& json) {
  auto begin = json.data();
  auto end = begin + json.size();
  begin = skip_spaces(begin, end);
  return skip_spaces(expect_json(begin, end), end) == end;
}

unittest("Validate json.") {
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

  assert_true(validate_json(json));
}

}
