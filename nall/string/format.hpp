#pragma once

namespace nall {

//nall::format is a vector<string> of parameters that can be applied to a string
//each {#} token will be replaced with its appropriate format parameter

inline auto string::format(const nall::string_format& params) -> type& {
  auto size = (int)this->size();
  auto data = memory::allocate<char>(size);
  memory::copy(data, this->data(), size);

  int x = 0;
  while(x < size - 2) {  //2 = minimum tag length
    if(data[x] != '{') { x++; continue; }

    int y = x + 1;
    while(y < size - 1) {  //-1 avoids going out of bounds on test after this loop
      if(data[y] != '}') { y++; continue; }
      break;
    }

    if(data[y++] != '}') { x++; continue; }

    static auto isNumeric = [](char* s, char* e) -> bool {
      if(s == e) return false;  //ignore empty tags: {}
      while(s < e) {
        if(*s >= '0' && *s <= '9') { s++; continue; }
        return false;
      }
      return true;
    };
    if(!isNumeric(&data[x + 1], &data[y - 1])) { x++; continue; }

    unsigned index = toNatural(&data[x + 1]);
    if(index >= params.size()) { x++; continue; }

    unsigned sourceSize = y - x;
    unsigned targetSize = params[index].size();
    unsigned remaining = size - x;

    if(sourceSize > targetSize) {
      unsigned difference = sourceSize - targetSize;
      memory::move(&data[x], &data[x + difference], remaining - difference);
      size -= difference;
    } else if(targetSize > sourceSize) {
      unsigned difference = targetSize - sourceSize;
      data = (char*)realloc(data, size + difference);
      size += difference;
      memory::move(&data[x + difference], &data[x], remaining);
    }
    memory::copy(&data[x], params[index].data(), targetSize);
    x += targetSize;
  }

  resize(size);
  memory::copy(get(), data, size);
  memory::free(data);
  return *this;
}

template<typename T, typename... P> inline auto string_format::append(const T& value, P&&... p) -> string_format& {
  vector<string>::append(value);
  return append(std::forward<P>(p)...);
}

inline auto string_format::append() -> string_format& {
  return *this;
}

template<typename... P> inline auto print(P&&... p) -> void {
  string s{std::forward<P>(p)...};
  fwrite(s.data(), 1, s.size(), stdout);
  fflush(stdout);
}

template<typename... P> inline auto print(FILE* fp, P&&... p) -> void {
  string s{std::forward<P>(p)...};
  fwrite(s.data(), 1, s.size(), fp);
  if(fp == stdout || fp == stderr) fflush(fp);
}

template<typename T> inline auto pad(const T& value, long precision, char padchar) -> string {
  string buffer{value};
  if(precision) buffer.size(precision, padchar);
  return buffer;
}

inline auto hex(uintmax value, long precision, char padchar) -> string {
  string buffer;
  buffer.resize(sizeof(uintmax) * 2);
  char* p = buffer.get();

  unsigned size = 0;
  do {
    unsigned n = value & 15;
    p[size++] = n < 10 ? '0' + n : 'a' + n - 10;
    value >>= 4;
  } while(value);
  buffer.resize(size);
  buffer.reverse();
  if(precision) buffer.size(precision, padchar);
  return buffer;
}

inline auto octal(uintmax value, long precision, char padchar) -> string {
  string buffer;
  buffer.resize(sizeof(uintmax) * 3);
  char* p = buffer.get();

  unsigned size = 0;
  do {
    p[size++] = '0' + (value & 7);
    value >>= 3;
  } while(value);
  buffer.resize(size);
  buffer.reverse();
  if(precision) buffer.size(precision, padchar);
  return buffer;
}

inline auto binary(uintmax value, long precision, char padchar) -> string {
  string buffer;
  buffer.resize(sizeof(uintmax) * 8);
  char* p = buffer.get();

  unsigned size = 0;
  do {
    p[size++] = '0' + (value & 1);
    value >>= 1;
  } while(value);
  buffer.resize(size);
  buffer.reverse();
  if(precision) buffer.size(precision, padchar);
  return buffer;
}

}
