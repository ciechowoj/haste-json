#include <haste/test>
#include <haste/json.hpp>

using namespace haste;
using namespace haste::mark2;

unittest("A segmentation fault occurs on de-serialization of canada.json.") {



  assert_eq(1.0, from_json<double>(to_json(1.0)));




}

