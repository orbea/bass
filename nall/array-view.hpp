#pragma once

namespace nall {

template<typename T> struct array_view {
  using type = array_view;

  array_view() {
    _data = nullptr;
    _size = 0;
  }

  array_view(std::nullptr_t) {
    _data = nullptr;
    _size = 0;
  }

  array_view(const void* data, uint64_t size) {
    _data = (const T*)data;
    _size = (int)size;
  }

  explicit operator bool() const { return _data && _size > 0; }

  operator const T*() const {
    return _data;
  }

  auto operator++() -> type& { _data++; _size--; return *this; }
  auto operator--() -> type& { _data--; _size++; return *this; }

  auto operator++(int) -> type { auto copy = *this; ++(*this); return copy; }
  auto operator--(int) -> type { auto copy = *this; --(*this); return copy; }

  auto operator-=(int distance) -> type& { _data -= distance; _size += distance; return *this; }
  auto operator+=(int distance) -> type& { _data += distance; _size -= distance; return *this; }

  auto operator[](unsigned index) const -> const T& {
    return _data[index];
  }

  auto operator()(unsigned index, const T& fallback = {}) const -> T {
    if(index >= _size) return fallback;
    return _data[index];
  }

  template<typename U = T> auto data() const -> const U* { return (const U*)_data; }
  template<typename U = T> auto size() const -> uint64_t { return _size * sizeof(T) / sizeof(U); }

  auto begin() const -> iterator_const<T> { return {_data, (unsigned)0}; }
  auto end() const -> iterator_const<T> { return {_data, (unsigned)_size}; }

  auto rbegin() const -> reverse_iterator_const<T> { return {_data, (unsigned)_size - 1}; }
  auto rend() const -> reverse_iterator_const<T> { return {_data, (unsigned)-1}; }

  auto read() -> T {
    auto value = operator[](0);
    _data++;
    _size--;
    return value;
  }

  auto view(unsigned offset, unsigned length) const -> type {
    return {_data + offset, length};
  }

  //array_view<uint8_t> specializations
  template<typename U> auto readl(U& value, unsigned size) -> U;
  template<typename U> auto readm(U& value, unsigned size) -> U;
  template<typename U> auto readvn(U& value, unsigned size) -> U;
  template<typename U> auto readvi(U& value, unsigned size) -> U;

  template<typename U> auto readl(U& value, unsigned offset, unsigned size) -> U { return view(offset, size).readl(value, size); }

  template<typename U = uint64_t> auto readl(unsigned size) -> U { U value; return readl(value, size); }
  template<typename U = uint64_t> auto readm(unsigned size) -> U { U value; return readm(value, size); }
  template<typename U = uint64_t> auto readvn(unsigned size) -> U { U value; return readvn(value, size); }
  template<typename U =  int64_t> auto readvi(unsigned size) -> U { U value; return readvi(value, size); }

  template<typename U = uint64_t> auto readl(unsigned offset, unsigned size) -> U { U value; return readl(value, offset, size); }

protected:
  const T* _data;
  int _size;
};

//array_view<uint8_t>

template<> template<typename U> inline auto array_view<uint8_t>::readl(U& value, unsigned size) -> U {
  value = 0;
  for(unsigned byte : range(size)) value |= (U)read() << byte * 8;
  return value;
}

template<> template<typename U> inline auto array_view<uint8_t>::readm(U& value, unsigned size) -> U {
  value = 0;
  for(unsigned byte : reverse(range(size))) value |= (U)read() << byte * 8;
  return value;
}

template<> template<typename U> inline auto array_view<uint8_t>::readvn(U& value, unsigned size) -> U {
  value = 0;
  unsigned shift = 1;
  while(true) {
    auto byte = read();
    value += (byte & 0x7f) * shift;
    if(byte & 0x80) break;
    shift <<= 7;
    value += shift;
  }
  return value;
}

template<> template<typename U> inline auto array_view<uint8_t>::readvi(U& value, unsigned size) -> U {
  value = readvn<U>();
  bool negate = value & 1;
  value >>= 1;
  if(negate) value = ~value;
  return value;
}

}
