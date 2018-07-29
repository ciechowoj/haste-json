#include <haste/test>
#include <haste/json.hpp>
#include <double-conversion/double-conversion.h>
#include <vector>
#include <iostream>
#include <cctype>

using namespace std;

namespace haste::mark2 {

void appender_t::push(char x) {
  _storage.push_back(x);
}

void appender_t::push(const char* x) {
  _storage.append(x);
}

void appender_t::push(const char* i, const char* e) {
  _storage.append(i, e);
}

void appender_t::push_key(string_view key) {
  _storage.push_back('"');
  _storage.append(key);
  _storage.append("\":");
}

void appender_t::push_comma_key(string_view key) {
  _storage.append(",\"");
  _storage.append(key);
  _storage.append("\":");
}

string appender_t::str() const {
  return _storage;
}

inline void skip_spaces(string_view& json) {
  while (!json.empty() && std::isspace(json.front())) {
    json.remove_prefix(1);
  }
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

inline void skip_string(string_view& json, const char* str) {
  auto rollback = json;

  while (!json.empty() && *str != '\0' && json.front() == *str) {
    json.remove_prefix(1);
    ++str;
  }

  if (*str != '\0') {
    json = rollback;
  }
}

inline void skip_string(string_view& json) {
  while (!json.empty() && json.front() != '"') {
    json.remove_prefix(1);
  }
}

inline void expect_char(string_view& json, char chr) {
  if (!json.empty() && json.front() == chr) {
    json.remove_prefix(1);
  }
  else {
    throw std::runtime_error(chr + std::string(" expected!"));
  }
}

inline const char* skip_char(const char* itr, const char* end, char chr) {
  if (itr < end && *itr == chr) {
    return itr + 1;
  }

  return itr;
}

inline string_view expect_string_literal(string_view& json) {
  expect_char(json, '"');
  auto begin = json.data();

  while (!json.empty() && json.front() != '"') {
    json.remove_prefix(1);
  }

  if (!json.empty()) {
    auto end = json.data();
    json.remove_prefix(1);
    return string_view(begin, end - begin);
  }
  else {
    throw std::runtime_error("String literal expected!");
  }
}

inline void expect_json(string_view& json) {
  if (!json.empty()) {
    if (json.front() == '{') {
      json.remove_prefix(1);

      while (!json.empty() && json.front() != '}') {
        skip_spaces(json);
        expect_string_literal(json);
        skip_spaces(json);
        expect_char(json, ':');
        skip_spaces(json);
        expect_json(json);
        skip_spaces(json);

        if (json.front() != '}') {
          expect_char(json, ',');
        }
      }

      expect_char(json, '}');
    }
    else if (json.front() == '[') {
      json.remove_prefix(1);
    }
    else if (isdigit(json.front())) {
      json.remove_prefix(1);

      while (!json.empty() && isdigit(json.front())) {
        json.remove_prefix(1);
      }
    }
    else {
      throw std::runtime_error("Json expected!");
    }
  }
}

void json_list_ref::from_json(string_view& json) const {
  expect_char(json, '[');

  auto list_itr = (char*)_begin;
  auto list_end = (char*)_end;

  while (!json.empty() && json.front() != ']') {
    if (list_itr == list_end) {
      throw std::runtime_error("list is too long");
    }

    skip_spaces(json);
    _cbck(json, list_itr);
    skip_spaces(json);

    if (json.front() != ',') {
      skip_spaces(json);
      break;
    }
    else {
      json.remove_prefix(1);
      skip_spaces(json);
    }

    list_itr += _item_size;
  }

  return expect_char(json, ']');
}

void json_list_cref::to_json(appender_t& appender) const {
  appender.push('[');

  auto itr = (const char*)_begin;
  auto end = (const char*)_end;

  if (itr < end) {
    _cbck(appender, itr);
    itr += _item_size;
  }

  while (itr < end) {
    appender.push(',');
    _cbck(appender, itr);
    itr += _item_size;
  }

  appender.push(']');

}

void json_property_ref::from_json(string_view& json) {
  _cbck(json, _data);
}

void json_property_cref::to_json(appender_t& appender) const {
  _cbck(appender, _data);
}

void json_convert_n::from_json(
  string_view& json,
  void* container,
  void (*cbck)(string_view&, void*)) {
  expect_char(json, '[');

  while (!json.empty() && json.front() != ']') {
    skip_spaces(json);
    cbck(json, container);
    skip_spaces(json);

    if (json.front() != ',') {
      skip_spaces(json);
      break;
    }
    else {
      json.remove_prefix(1);
      skip_spaces(json);
    }
  }

  expect_char(json, ']');
}

void json_convert_n::from_json(string_view& json, json_property_ref* prop_begin, json_property_ref* prop_end) {
  skip_spaces(json);
  expect_char(json, '{');

  while (!json.empty() && json.front() != '}') {
    skip_spaces(json);
    auto literal = json;
    expect_char(json, '"');

    auto prop_itr = prop_begin;

    for (; prop_itr < prop_end; ++prop_itr) {
      auto temp = json;
      skip_string(temp, prop_itr->name);

      if (json != temp) {
        json = temp;
        break;
      }
    }

    if (prop_itr == prop_end) {
      expect_string_literal(literal);
      json = literal;
      skip_spaces(json);
      expect_char(json, ':');
      skip_spaces(json);
      expect_json(json);

    }
    else {
      expect_char(json, '"');
      skip_spaces(json);
      expect_char(json, ':');
      skip_spaces(json);

      if (json.empty()) {
        throw std::runtime_error("Json error!");
      }
      else {
        prop_itr->from_json(json);
      }
    }

    skip_spaces(json);

    if (json.front() != ',') {
      skip_spaces(json);
      break;
    }
    else {
      json.remove_prefix(1);
      skip_spaces(json);
    }
  }

  expect_char(json, '}');
}

void json_convert_n::to_json(appender_t& appender, const json_property_cref* itr, const json_property_cref* end) {
  appender.push('{');

  for (; itr != end; ++itr) {
    appender.push('"');
    appender.push(itr->name);
    appender.push("\":");

    itr->to_json(appender);

    if (itr + 1 != end) {
      appender.push(',');
    }
  }

  appender.push('}');
}

void dict_from_json(
  string_view& json,
  void* erased,
  void (*cbck)(string_view&, string_view, void*)) {
  expect_char(json, '{');

  while (!json.empty() && json.front() != '}') {
    skip_spaces(json);
    auto key = expect_string_literal(json);
    skip_spaces(json);
    expect_char(json, ':');

    cbck(json, key, erased);
    skip_spaces(json);

    if (json.front() != ',') {
      skip_spaces(json);
      break;
    }
    else {
      json.remove_prefix(1);
      skip_spaces(json);
    }
  }

  expect_char(json, '}');
}

void json_convert<int>::from_json(
  string_view& json,
  int& x) {
  auto rollback = json;
  int result = 0;
  int sign = 1;

  if (!json.empty() && json.front() == '-') {
    sign = -1;
    json.remove_prefix(1);
  }

  while (!json.empty() && isdigit(json.front())) {
    result *= 10;
    result += json.front() - '0';
    json.remove_prefix(1);
  }

  if (json != rollback) {
    x = result * sign;
  }
}

void json_convert<int>::to_json(appender_t& appender, int x) {
  char buffer[12];
  int num = sprintf(buffer, "%d", x);
  appender.push(buffer, buffer + num);
}

static const auto from_converter =
  double_conversion::
    StringToDoubleConverter(
      double_conversion::StringToDoubleConverter::ALLOW_TRAILING_JUNK,
      0.0, 0.0, nullptr, nullptr);

void json_convert<double>::from_json(string_view& json, double& x) {
  int processed_characters_count = 0;
  x = from_converter.StringToDouble(json.data(), json.size(), &processed_characters_count);
  json.remove_prefix(processed_characters_count);
}

static const auto to_converter =
  double_conversion::
    DoubleToStringConverter(0, nullptr, nullptr, 'e', -7, 7, 7, 7);

void json_convert<double>::to_json(appender_t& appender, double x) {
  char buffer[32];
  auto builder = double_conversion::StringBuilder(buffer, sizeof(buffer));
  to_converter.ToShortest(x, &builder);
  appender.push(builder.Finalize());
}

void json_convert<std::string>::from_json(
  string_view& json,
  std::string& x) {


  skip_spaces(json);
  expect_char(json, '"');

  while (!json.empty() && json.front() != '"') {
    x.push_back(json.front());
    json.remove_prefix(1);
  }

  expect_char(json, '"');
}

void json_convert<std::string>::to_json(appender_t& appender, const std::string& x) {
  appender.push("\"");
  appender.push(x.c_str());
  appender.push("\"");
}

string read_file(const string& path) {
  FILE* file = std::fopen(path.c_str(), "rb");

  if(!file) {
    throw std::runtime_error("Failed to open file \"" + path + "\".");
  }

  std::fseek(file, 0, SEEK_END);
  std::size_t size = std::ftell(file);
  std::fseek(file, 0, SEEK_SET);

  string result;
  result.resize(size);

  std::fread(result.data(), sizeof(uint8_t), result.size(), file);
  std::fclose(file);

  return result;
}

}
