#include <haste/json.hpp>
#include <haste/test>
#include <iostream>

using namespace haste::mark2;
using namespace std;

struct properties_t {
  JSON_PROPERTY("name", string) name;
};

using coordinates_t = vector<vector<array<double, 2>>>;

struct geometry_t {
  JSON_PROPERTY("type", string) type;
  JSON_PROPERTY("coordinates", coordinates_t) coordinates;
};

struct feature_t {
  JSON_PROPERTY("type", string) type;
  JSON_PROPERTY("properties", properties_t) properties;
  JSON_PROPERTY("geometry", geometry_t) geometry;
};

struct collection_t {
  JSON_PROPERTY("type", string) type;
  JSON_PROPERTY("features", vector<feature_t>) features;
};

unittest() {
  auto actual = from_json<collection_t>(read_file("data/canada.json"));
}
