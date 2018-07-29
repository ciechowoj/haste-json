#include <haste/test>
#include <haste/json.hpp>
#include <iostream>

using namespace haste;
using namespace haste::mark2;
using std::string;

int main() {
  return haste::run_all_tests() ? 0 : 1;
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
  static_assert(has_json_property<dummy_struct_with_string>);
  static_assert(!has_json_property<std::vector<int>>);

  static_assert(std::is_standard_layout<std::vector<int>>::value);
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
  assert_eq(-123456789, from_json<int>("-123456789"));
}

unittest("Array serialization.") {
  assert_eq(vector<int> { 1, 2, 3, 4, 5 }, from_json<vector<int>>("[1,2,3,4,5]"));
  assert_eq("[1,2,3,4,5]", to_json(vector<int> { 1, 2, 3, 4, 5 }));
}


struct date_t {
  JSON_PROPERTY("day", int) day;
  JSON_PROPERTY("month", int) month;
  JSON_PROPERTY("year", int) year;
};

struct employee_t {
  JSON_PROPERTY("name", std::string) name;
  JSON_PROPERTY("employed_date", date_t) employed_date;
  JSON_PROPERTY("salary", int) salary;
  JSON_PROPERTY("level", std::string) level;
};

struct team_t {
  JSON_PROPERTY("name", std::string) name;
  JSON_PROPERTY("employees", std::vector<employee_t>) employees;
};

unittest("Serialize team description.") {
  team_t highly_trained_monkeys {
    "Highly Trained Monkeys",
    vector<employee_t> {
      { "Maxymilian Debeściak", { 10, 12, 1999 }, 13000, "Software Engineer" },
      { "Geralt of Rivia", { 13, 3, 1742 }, 12345, "QA Engineer / Witcher" },
      { "Jakub Wyndrowycz", { 1, 1, 1900 }, 100, "Amateur Exorcist" }
    }
  };

  const char* expected = R"foo(
  {
    "name": "Highly Trained Monkeys",
    "employees":
    [
      {
        "name": "Maxymilian Debeściak",
        "employed_date": { "day": 10, "month": 12, "year": 1999},
        "salary": 13000, "level": "Software Engineer"
      },
      {
        "name": "Geralt of Rivia",
        "employed_date": { "day": 13, "month": 3, "year": 1742},
        "salary": 12345,
        "level": "QA Engineer / Witcher"
      },
      {
        "name": "Jakub Wyndrowycz",
        "employed_date": { "day": 1, "month": 1,"year": 1900},
        "salary": 100,
        "level": "Amateur Exorcist"
      }
    ]
  })foo";

  assert_almost_eq(expected, to_json(highly_trained_monkeys));
}

unittest("De-serialize team description.") {
  auto team = from_json<team_t>(R"foo(
  {
    "name": "Highly Trained Monkeys",
    "employees":
    [
      {
        "name": "Maxymilian Debeściak",
        "employed_date": { "day": 10, "month": 12, "year": 1999},
        "salary": 13000, "level": "Software Engineer"
      }
    ]
  })foo");

  assert_eq(team.name, "Highly Trained Monkeys");
  assert_eq(team.employees.size(), 1u);
  assert_eq(team.employees[0].name, "Maxymilian Debeściak");
  assert_eq(team.employees[0].employed_date.day, 10);
  assert_eq(team.employees[0].salary, 13000);
  assert_eq(team.employees[0].level, "Software Engineer");
}

unittest("No runtime space overhead.") {
  static_assert(sizeof(date_t) == sizeof(int) * 3);
  static_assert(sizeof(employee_t) == sizeof(std::string) * 2 + sizeof(date_t) + sizeof(int));
}



