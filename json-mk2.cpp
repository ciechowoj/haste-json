#include <haste/test>
#include <haste/json-mk2>

#include <vector>
#include <iostream>
#include <cctype>

using namespace std;

namespace haste::mark2 {

struct test_struct_t : public json_contract {
  JSON_PROPERTY("x", int) x;
};

unittest() {

  to_json(0);
  from_json<int>("0");

  test_struct_t x;
  to_json(x);
  from_json<test_struct_t>("0");

  to_json(vector<int>());
}



}
