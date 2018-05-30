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

}
