#include <haste/test>
#include <haste/json-mk2>

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

string appender_t::str() const {
  return _storage;
}

const char* json_property_ref::name() const {
  return _name;
}

const char* json_property_ref::from_json(const char* itr, const char* end) {
  return _cbck(itr, end, _data);
}

const char* json_property_cref::name() const {
  return _name;
}

void json_property_cref::to_json(appender_t& appender) const {
  _cbck(appender, _data);
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

const char* json_convert_n::from_json(const char* itr, const char* end, json_property_ref* prop_begin, json_property_ref* prop_end) {
  itr = skip_spaces(itr, end);
  itr = expect_char(itr, end, '{');

  while (itr < end && *itr != '}') {
    itr = skip_spaces(itr, end);
    itr = expect_char(itr, end, '"');

    auto prop_itr = prop_begin;

    for (; prop_itr < prop_end; ++prop_itr) {
      auto jtr = skip_string(itr, end, prop_itr->name());

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
    appender.push(itr->name());
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

  while (itr < end && isdigit(*itr)) {
    result *= 10;
    result += *itr - '0';
    ++itr;
  }

  if (itr != rollback) {
    x = result;
  }

  return itr;
}

void json_convert<int>::to_json(appender_t& appender, int x) {
  if (x == 0) {
    appender.push('0');
  }
  else {
    appender.push(x + '0');
  }
}

}
