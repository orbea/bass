#pragma once

namespace nall::memory {
  template<typename T = uint8_t> auto allocate(unsigned size) -> T*;
  template<typename T = uint8_t> auto allocate(unsigned size, const T& value) -> T*;

  template<typename T = uint8_t> auto resize(void* target, unsigned size) -> T*;

  auto free(void* target) -> void;

  template<typename T = uint8_t> auto compare(const void* target, unsigned capacity, const void* source, unsigned size) -> int;
  template<typename T = uint8_t> auto compare(const void* target, const void* source, unsigned size) -> int;

  template<typename T = uint8_t> auto icompare(const void* target, unsigned capacity, const void* source, unsigned size) -> int;
  template<typename T = uint8_t> auto icompare(const void* target, const void* source, unsigned size) -> int;

  template<typename T = uint8_t> auto copy(void* target, unsigned capacity, const void* source, unsigned size) -> T*;
  template<typename T = uint8_t> auto copy(void* target, const void* source, unsigned size) -> T*;

  template<typename T = uint8_t> auto move(void* target, unsigned capacity, const void* source, unsigned size) -> T*;
  template<typename T = uint8_t> auto move(void* target, const void* source, unsigned size) -> T*;

  template<typename T = uint8_t> auto fill(void* target, unsigned capacity, const T& value = {}) -> T*;

  template<typename T> auto assign(T* target) -> void {}
  template<typename T, typename U, typename... P> auto assign(T* target, const U& value, P&&... p) -> void;

  template<unsigned size, typename T = uint64_t> auto readl(const void* source) -> T;
  template<unsigned size, typename T = uint64_t> auto readm(const void* source) -> T;

  template<unsigned size, typename T = uint64_t> auto writel(void* target, T data) -> void;
  template<unsigned size, typename T = uint64_t> auto writem(void* target, T data) -> void;
}

namespace nall::memory {

//implementation notes:
//memcmp, memcpy, memmove have terrible performance on small block sizes (FreeBSD 10.0-amd64)
//as this library is used extensively by nall/string, and most strings tend to be small,
//this library hand-codes these functions instead. surprisingly, it's a substantial speedup

template<typename T> auto allocate(unsigned size) -> T* {
  return (T*)malloc(size * sizeof(T));
}

template<typename T> auto allocate(unsigned size, const T& value) -> T* {
  auto result = allocate<T>(size);
  if(result) fill<T>(result, size, value);
  return result;
}

template<typename T> auto resize(void* target, unsigned size) -> T* {
  return (T*)realloc(target, size * sizeof(T));
}

inline auto free(void* target) -> void {
  ::free(target);
}

template<typename T> auto compare(const void* target, unsigned capacity, const void* source, unsigned size) -> int {
  auto t = (uint8_t*)target;
  auto s = (uint8_t*)source;
  auto l = std::min(capacity, size) * sizeof(T);
  while(l--) {
    auto x = *t++;
    auto y = *s++;
    if(x != y) return x - y;
  }
  if(capacity == size) return 0;
  return -(capacity < size);
}

template<typename T> auto compare(const void* target, const void* source, unsigned size) -> int {
  return compare<T>(target, size, source, size);
}

template<typename T> auto icompare(const void* target, unsigned capacity, const void* source, unsigned size) -> int {
  auto t = (uint8_t*)target;
  auto s = (uint8_t*)source;
  auto l = std::min(capacity, size) * sizeof(T);
  while(l--) {
    auto x = *t++;
    auto y = *s++;
    if(x - 'A' < 26) x += 32;
    if(y - 'A' < 26) y += 32;
    if(x != y) return x - y;
  }
  return -(capacity < size);
}

template<typename T> auto icompare(const void* target, const void* source, unsigned size) -> int {
  return icompare<T>(target, size, source, size);
}

template<typename T> auto copy(void* target, unsigned capacity, const void* source, unsigned size) -> T* {
  auto t = (uint8_t*)target;
  auto s = (uint8_t*)source;
  auto l = std::min(capacity, size) * sizeof(T);
  while(l--) *t++ = *s++;
  return (T*)target;
}

template<typename T> auto copy(void* target, const void* source, unsigned size) -> T* {
  return copy<T>(target, size, source, size);
}

template<typename T> auto move(void* target, unsigned capacity, const void* source, unsigned size) -> T* {
  auto t = (uint8_t*)target;
  auto s = (uint8_t*)source;
  auto l = std::min(capacity, size) * sizeof(T);
  if(t < s) {
    while(l--) *t++ = *s++;
  } else {
    t += l;
    s += l;
    while(l--) *--t = *--s;
  }
  return (T*)target;
}

template<typename T> auto move(void* target, const void* source, unsigned size) -> T* {
  return move<T>(target, size, source, size);
}

template<typename T> auto fill(void* target, unsigned capacity, const T& value) -> T* {
  auto t = (T*)target;
  while(capacity--) *t++ = value;
  return (T*)target;
}

template<typename T, typename U, typename... P> auto assign(T* target, const U& value, P&&... p) -> void {
  *target++ = value;
  assign(target, std::forward<P>(p)...);
}

template<unsigned size, typename T> auto readl(const void* source) -> T {
  auto p = (const uint8_t*)source;
  T data = 0;
  for(unsigned n = 0; n < size; n++) data |= T(*p++) << n * 8;
  return data;
}

template<unsigned size, typename T> auto readm(const void* source) -> T {
  auto p = (const uint8_t*)source;
  T data = 0;
  for(int n = size - 1; n >= 0; n--) data |= T(*p++) << n * 8;
  return data;
}

template<unsigned size, typename T> auto writel(void* target, T data) -> void {
  auto p = (uint8_t*)target;
  for(unsigned n = 0; n < size; n++) *p++ = data >> n * 8;
}

template<unsigned size, typename T> auto writem(void* target, T data) -> void {
  auto p = (uint8_t*)target;
  for(int n = size - 1; n >= 0; n--) *p++ = data >> n * 8;
}

}
