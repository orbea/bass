#pragma once

namespace nall {

template<typename T> struct array;

//usage: int x[256] => array<int[256]> x
template<typename T, unsigned Size> struct array<T[Size]> {
  array() = default;

  array(const std::initializer_list<T>& source) {
    unsigned index = 0;
    for(auto& value : source) {
      operator[](index++) = value;
    }
  }

  operator array_span<T>() {
    return {data(), size()};
  }

  operator array_view<T>() const {
    return {data(), size()};
  }

  alwaysinline auto operator[](unsigned index) -> T& {
    return values[index];
  }

  alwaysinline auto operator[](unsigned index) const -> const T& {
    return values[index];
  }

  alwaysinline auto operator()(unsigned index, const T& fallback = {}) const -> const T& {
    if(index >= Size) return fallback;
    return values[index];
  }

  auto fill(const T& fill = {}) -> array& {
    for(auto& value : values) value = fill;
    return *this;
  }

  auto data() -> T* { return values; }
  auto data() const -> const T* { return values; }
  auto size() const -> unsigned { return Size; }

  auto begin() -> T* { return &values[0]; }
  auto end() -> T* { return &values[Size]; }

  auto begin() const -> const T* { return &values[0]; }
  auto end() const -> const T* { return &values[Size]; }

private:
  T values[Size];
};

template<typename T, T... p> inline auto from_array(unsigned index) -> T {
  static const array<T[sizeof...(p)]> table{p...};
  struct out_of_bounds {};
  return table[index];
}

template<int64_t... p> inline auto from_array(unsigned index) -> int64_t {
  static const array<int64_t[sizeof...(p)]> table{p...};
  struct out_of_bounds {};
  return table[index];
}

}
