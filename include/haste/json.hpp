#pragma once
#include <string>
#include <vector>
#include <array>
#include <map>
#include <type_traits>
#include <haste/fields_count.hpp>
#include <utility>
#include <string_view>

namespace haste::mark2 {

using std::string;
using std::vector;
using boost::pfr::detail::fields_count;
using std::string_view;

using std::enable_if_t;
using namespace std;

class appender_t {
public:
  void push(char x);
  void push(const char* x);
  void push(const string& x) { push(x.c_str()); }
  void push(const char* i, const char* e);

  void push_key(string_view key);
  void push_comma_key(string_view key);

  std::string str() const;
private:
  string _storage;
};

template <auto& X, class T> struct json_property : public T {
  constexpr static const char* name = X;

  public:
    json_property& operator=(const T& that) {
      *this = that;
      return *this;
    }
};

template <auto& X> struct json_property<X, int> {
  constexpr static const char* name = X;
  int value;

  public:
    operator int() { return value; }

    json_property& operator=(const int& that) {
      value = that;
      return *this;
    }
};

template <auto& X, class T> struct json_property<X, T[]> {
  constexpr static const char* name = X;
  int value;

  public:
    operator int() { return value; }

    json_property& operator=(const int& that) {
      value = that;
      return *this;
    }
};

template <class T, class Enable = void> struct json_convert {
  static_assert("Unsupported type!");
};

template <class T> struct is_json_property {
  constexpr static bool value = false;
};

template <auto& X, class T> struct is_json_property<json_property<X, T>> {
  constexpr static bool value = true;
};


template <int N, class T> struct has_json_property_at_n {

  template <int M> struct helper {
    template <class U, typename std::enable_if_t<M != N || is_json_property<U>::value, int> = 0> constexpr operator U&();
  };


  template <std::size_t... Is, class Q = decltype(T { helper<Is>{} ...  })>
    static constexpr auto eval(std::index_sequence<Is...>) { return true; }

  static constexpr auto eval(...) { return false; }

  static constexpr bool value = eval(std::make_index_sequence<fields_count<T>()>());
};

template <class T, class Enable = void> struct has_json_property_t {
  static constexpr bool value = false;
};

template <class T> struct has_json_property_t<T, typename std::enable_if_t<std::is_aggregate<T>::value>> {
  static constexpr int N = fields_count<T>();

  template <std::size_t... Is>
    static constexpr auto eval(std::index_sequence<Is...>) {
      return (has_json_property_at_n<Is, T>::value || ...);
    }

  static constexpr bool value = eval(std::make_index_sequence<N>());
};


template <class T> constexpr bool has_json_property = has_json_property_t<T>::value;
template <class T> constexpr bool is_json_struct = has_json_property_t<T>::value;

struct json_ref_base {
  template <class T> json_ref_base(T*) {
    this->_cbck = [](string_view& json, void* p) {
      json_convert<T>::from_json(json, *((T*)p));
    };
  }

protected:
  void (*_cbck)(string_view&, void*);
};

struct json_cref_base {
  template <class T> json_cref_base(const T*) {
    this->_cbck = [](appender_t& a, const void* p) {
      return json_convert<T>::to_json(a, *((const T*)p));
    };
  }

protected:
  void (*_cbck)(appender_t&, const void*);
};

struct json_list_ref : private json_ref_base {
  template <class T> json_list_ref(T* begin, T* end)
    : json_ref_base(begin)
    , _item_size(sizeof(T))
    , _begin(begin)
    , _end(end) { }

  void from_json(string_view& json) const;
private:
  size_t _item_size;
  void *_begin, *_end;
};

struct json_list_cref : private json_cref_base {
  template <class T> json_list_cref(const T* begin, const T* end)
    : json_cref_base(begin)
    , _item_size(sizeof(T))
    , _begin(begin)
    , _end(end) { }

  void to_json(appender_t& appender) const;
private:
  size_t _item_size;
  const void *_begin, *_end;
};

struct json_property_ref {
  template <auto& X, class T> json_property_ref(json_property<X, T>& value)
    : name(value.name) {
    this->_data = &value;
    this->_cbck = [](string_view& json, void* p) {
      json_convert<T>::from_json(json, *((T*)p));
    };
  }

  void from_json(string_view& json);
  char const* const name;
private:
  void* _data;
  void (*_cbck)(string_view& json, void*);
};

struct json_property_cref {
  template <auto& X, class T> json_property_cref(const json_property<X, T>& value)
    : name(value.name) {
    this->_data = &value;
    this->_cbck = [](appender_t& a, const void* p) {
      json_convert<T>::to_json(a, *((const T*)p));
    };
  }

  void to_json(appender_t& appender) const;
  char const* const name;
private:
  const void* _data;
  void (*_cbck)(appender_t&, const void*);
};

struct json_convert_n {
  static void from_json(string_view&, void*, void (*)(string_view&, void*));
  static void from_json(string_view&, json_property_ref*, json_property_ref*);
  static void to_json(appender_t& appender, const json_property_cref*, const json_property_cref*);
};

void from_json(const char*&, const char*, void*, void (*)(string_view&, string_view, void*));

template <class T> struct json_convert<T, typename std::enable_if_t<has_json_property<T>> > {
  constexpr static int N = fields_count<std::decay_t<T>>();

  static void from_json(string_view& json, T& x) {
    auto unpacked = unpack<json_property_ref>(x);
    return json_convert_n::from_json(json, unpacked.data(), unpacked.data() + unpacked.size());
  }

  static void to_json(appender_t& appender, const T& x) {
    auto unpacked = unpack<json_property_cref>(x);
    json_convert_n::to_json(appender, unpacked.data(), unpacked.data() + unpacked.size());
  }

  template <class C, class... U> static std::array<C, N> A(U&... x) {
    return std::array { C(x)... };
  }

  template <class C, class U> static std::array<C, N> unpack(U&& _v) {
         if constexpr (N == 0) { return A<C>(); }
    else if constexpr (N == 1) { auto&& [a] = _v; return A<C>(a); }
    else if constexpr (N == 2) { auto&& [a,b] = _v; return A<C>(a,b); }
    else if constexpr (N == 3) { auto&& [a,b,c] = _v; return A<C>(a,b,c); }
    else if constexpr (N == 4) { auto&& [a,b,c,d] = _v; return A<C>(a,b,c,d); }
    else if constexpr (N == 5) { auto&& [a,b,c,d,e] = _v; return A<C>(a,b,c,d,e); }
    else if constexpr (N == 6) { auto&& [a,b,c,d,e,f] = _v; return A<C>(a,b,c,d,e,f); }
    else if constexpr (N == 7) { auto&& [a,b,c,d,e,f,g] = _v; return A<C>(a,b,c,d,e,f,g); }
    else if constexpr (N == 8) { auto&& [a,b,c,d,e,f,g,h] = _v; return A<C>(a,b,c,d,e,f,g,h); }
    else if constexpr (N == 9) { auto&& [a,b,c,d,e,f,g,h,i] = _v; return A<C>(a,b,c,d,e,f,g,h,i); }
    else if constexpr (N == 10) { auto&& [a,b,c,d,e,f,g,h,i,j] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j); }
    else if constexpr (N == 11) { auto&& [a,b,c,d,e,f,g,h,i,j,k] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k); }
    else if constexpr (N == 12) { auto&& [a,b,c,d,e,f,g,h,i,j,k,l] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k,l); }
    else if constexpr (N == 13) { auto&& [a,b,c,d,e,f,g,h,i,j,k,l,m] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k,l,m); }
    else if constexpr (N == 14) { auto&& [a,b,c,d,e,f,g,h,i,j,k,l,m,n] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k,l,m,n); }
    else if constexpr (N == 15) { auto&& [a,b,c,d,e,f,g,h,i,j,k,l,m,n,o] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o); }
    else if constexpr (N == 16) { auto&& [a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p] = _v; return A<C>(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p); }
  }
};

template <> struct json_convert<int> {
  static void from_json(string_view& json, int& x);
  static void to_json(appender_t& appender, int x);
};

template <> struct json_convert<double> {
  static void from_json(string_view& json, double& x);
  static void to_json(appender_t& appender, double x);
};

template <> struct json_convert<std::string> {
  static void from_json(string_view& json, std::string& x);
  static void to_json(appender_t& appender, const std::string& x);
};

template <class T> struct json_convert<vector<T>> {
  static void from_json(string_view& json, vector<T>& x) {
    return json_convert_n::from_json(
      json,
      &x,
      [](string_view& json, void* c) {
        auto container = ((vector<T>*)c);
        container->emplace_back();
        json_convert<T>::from_json(json, container->back());
      });
  }

  static void to_json(appender_t& appender, const vector<T>& x) {
    json_list_cref(x.data(), x.data() + x.size()).to_json(appender);
  }
};

template <class T> struct json_convert<map<string, T>> {
  static const char* from_json(const char* itr, const char* end, map<string, T>& x) {
    return json_convert_n::from_json(
      itr,
      end,
      &x,
      [](const char*& itr, const char* end, string_view key, void* erased) {
        auto container = ((map<string, T>*)erased);
        auto value = json_convert<T>::from_json(itr, end);
        container.insert(make_pair(key, value));
      });
  }

  static void to_json(appender_t& appender, const map<string, T>& dict) {
    appender.push("{");

    auto itr = dict.begin();
    auto end = dict.end();

    if (itr != end) {
      appender.push_key(itr->first);
      json_convert<T>::to_json(appender, itr->second);
      ++itr;
    }

    while (itr != end) {
      appender.push_comma_key(itr->first);
      json_convert<T>::to_json(appender, itr->second);
      ++itr;
    }

    appender.push("}");
  }
};


template <class T, size_t N> struct json_convert<array<T, N>> {
  static void from_json(string_view& json, array<T, N>& x) {
    json_list_ref(x.data(), x.data() + x.size()).from_json(json);
  }

  static void to_json(appender_t& appender, const array<T, N>& x) {
    json_list_cref(x.data(), x.data() + x.size()).to_json(appender);
  }
};

template <class T> string to_json(const T& x) {
  appender_t appender;
  json_convert<T>::to_json(appender, x);
  return appender.str();
}

template <class T> T from_json(const string& x) {
  T result;
  string_view view = x;
  json_convert<T>::from_json(view, result);
  return result;
}

string read_file(const string& path);

#define JSON_PROPERTY_CONCAT(pref, line) pref##line

#define JSON_PROPERTY_HELPER(line, name, type) \
  static constexpr char JSON_PROPERTY_CONCAT(__string_literal, line)[] = name; \
  json_property<JSON_PROPERTY_CONCAT(__string_literal, line), type>

#define JSON_PROPERTY(name, type) \
  JSON_PROPERTY_HELPER(__LINE__, name, type)

}
