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

string appender_t::str() const {
  return _storage;
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

const char* json_list_ref::from_json(const char* itr, const char* end) const {
  itr = expect_char(itr, end, '[');

  auto list_itr = (char*)_begin;
  auto list_end = (char*)_end;

  while (itr < end && *itr != ']') {
    if (list_itr == list_end) {
      throw std::runtime_error("list is too long");
    }

    itr = skip_spaces(itr, end);
    itr = _cbck(itr, end, list_itr);
    itr = skip_spaces(itr, end);

    if (*itr != ',') {
      itr = skip_spaces(itr, end);
      break;
    }
    else {
      ++itr;
      itr = skip_spaces(itr, end);
    }

    list_itr += _item_size;
  }

  return expect_char(itr, end, ']');
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

const char* json_property_ref::from_json(const char* itr, const char* end) {
  return _cbck(itr, end, _data);
}

void json_property_cref::to_json(appender_t& appender) const {
  _cbck(appender, _data);
}

const char* json_convert_n::from_json(
  const char* itr,
  const char* end,
  void* container,
  const char* (*cbck)(const char*, const char*, void*)) {
  itr = expect_char(itr, end, '[');

  while (itr < end && *itr != ']') {
    itr = skip_spaces(itr, end);
    itr = cbck(itr, end, container);
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

  return expect_char(itr, end, ']');
}

const char* json_convert_n::from_json(const char* itr, const char* end, json_property_ref* prop_begin, json_property_ref* prop_end) {
  itr = skip_spaces(itr, end);
  itr = expect_char(itr, end, '{');

  while (itr < end && *itr != '}') {
    itr = skip_spaces(itr, end);
    itr = expect_char(itr, end, '"');

    auto prop_itr = prop_begin;

    for (; prop_itr < prop_end; ++prop_itr) {
      auto jtr = skip_string(itr, end, prop_itr->name);

      if (itr != jtr) {
        itr = jtr;
        break;
      }
    }

    if (prop_itr == prop_end) {
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
      else {
        itr = prop_itr->from_json(itr, end);
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

const char* json_convert<int>::from_json(
  const char* itr,
  const char* end,
  int& x) {
  const char* rollback = itr;
  int result = 0;
  int sign = 1;

  if (itr < end && *itr == '-') {
    sign = -1;
    ++itr;
  }

  while (itr < end && isdigit(*itr)) {
    result *= 10;
    result += *itr - '0';
    ++itr;
  }

  if (itr != rollback) {
    x = result * sign;
  }

  return itr;
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

const char* json_convert<double>::from_json(const char* itr, const char* end, double& x) {
  int processed_characters_count = 0;
  x = from_converter.StringToDouble(itr, end - itr, &processed_characters_count);
  return itr + processed_characters_count;
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

const char* json_convert<std::string>::from_json(
  const char* itr,
  const char* end,
  std::string& x) {


  itr = skip_spaces(itr, end);
  itr = expect_char(itr, end, '"');

  while (itr < end && *itr != '"') {
    x.push_back(*itr++);
  }

  return expect_char(itr, end, '"');
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
