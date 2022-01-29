#pragma once

namespace nall {

template<typename T> struct array_span : array_view<T> {
  using type = array_span;
  using super = array_view<T>;

  array_span() {
    super::_data = nullptr;
    super::_size = 0;
  }

  array_span(std::nullptr_t) {
    super::_data = nullptr;
    super::_size = 0;
  }

  array_span(void* data, uint64_t size) {
    super::_data = (T*)data;
    super::_size = (int)size;
  }

  operator T*() { return (T*)super::operator const T*(); }

  auto operator[](unsigned index) -> T& { return (T&)super::operator[](index); }

  template<typename U = T> auto data() -> U* { return (U*)super::_data; }
  template<typename U = T> auto data() const -> const U* { return (const U*)super::_data; }

  auto begin() -> iterator<T> { return {(T*)super::_data, (unsigned)0}; }
  auto end() -> iterator<T> { return {(T*)super::_data, (unsigned)super::_size}; }

  auto rbegin() -> reverse_iterator<T> { return {(T*)super::_data, (unsigned)super::_size - 1}; }
  auto rend() -> reverse_iterator<T> { return {(T*)super::_data, (unsigned)-1}; }

  auto write(T value) -> void {
    operator[](0) = value;
    super::_data++;
    super::_size--;
  }

  auto span(unsigned offset, unsigned length) const -> type {
    return {super::_data + offset, length};
  }

  //array_span<uint8_t> specializations
  template<typename U> auto writel(U value, unsigned size) -> void;
  template<typename U> auto writem(U value, unsigned size) -> void;
  template<typename U> auto writevn(U value, unsigned size) -> void;
  template<typename U> auto writevi(U value, unsigned size) -> void;
};

//array_span<uint8_t>

template<> inline auto array_span<uint8_t>::write(uint8_t value) -> void {
  operator[](0) = value;
  _data++;
  _size--;
}

template<> template<typename U> inline auto array_span<uint8_t>::writel(U value, unsigned size) -> void {
  for(unsigned byte : range(size)) write(value >> byte * 8);
}

template<> template<typename U> inline auto array_span<uint8_t>::writem(U value, unsigned size) -> void {
  for(unsigned byte : reverse(range(size))) write(value >> byte * 8);
}

template<> template<typename U> inline auto array_span<uint8_t>::writevn(U value, unsigned size) -> void {
  while(true) {
    auto byte = value & 0x7f;
    value >>= 7;
    if(value == 0) return write(0x80 | byte);
    write(byte);
    value--;
  }
}

template<> template<typename U> inline auto array_span<uint8_t>::writevi(U value, unsigned size) -> void {
  bool negate = value < 0;
  if(negate) value = ~value;
  value = value << 1 | negate;
  writevn(value);
}

}
