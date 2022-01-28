#pragma once

//only allocators may access _data or modify _size and _capacity
//all other functions must use data(), size(), capacity()

namespace nall {

/*****
  adaptive allocator
  sizeof(string) == SSO + 8

  aggressively tries to avoid heap allocations
  small strings are stored on the stack
  large strings are shared via copy-on-write

  SSO alone is very slow on large strings due to copying
  SSO alone is very slightly faster than this allocator on small strings

  COW alone is very slow on small strings due to heap allocations
  COW alone is very slightly faster than this allocator on large strings

  adaptive is thus very fast for all string sizes
*****/

inline string::string() : _data(nullptr), _capacity(SSO - 1), _size(0) {
}

template<typename T>
inline auto string::get() -> T* {
  if(_capacity < SSO) return (T*)_text;
  if(*_refs > 1) _copy();
  return (T*)_data;
}

template<typename T>
inline auto string::data() const -> const T* {
  if(_capacity < SSO) return (const T*)_text;
  return (const T*)_data;
}

inline auto string::reset() -> type& {
  if(_capacity >= SSO && !--*_refs) memory::free(_data);
  _data = nullptr;
  _capacity = SSO - 1;
  _size = 0;
  return *this;
}

inline auto string::reserve(uint capacity) -> type& {
  if(capacity <= _capacity) return *this;
  capacity = bit::round(capacity + 1) - 1;
  if(_capacity < SSO) {
    _capacity = capacity;
    _allocate();
  } else if(*_refs > 1) {
    _capacity = capacity;
    _copy();
  } else {
    _capacity = capacity;
    _resize();
  }
  return *this;
}

inline auto string::resize(uint size) -> type& {
  reserve(size);
  get()[_size = size] = 0;
  return *this;
}

inline auto string::operator=(const string& source) -> type& {
  if(&source == this) return *this;
  reset();
  if(source._capacity >= SSO) {
    _data = source._data;
    _refs = source._refs;
    _capacity = source._capacity;
    _size = source._size;
    ++*_refs;
  } else {
    memory::copy(_text, source._text, SSO);
    _capacity = source._capacity;
    _size = source._size;
  }
  return *this;
}

inline auto string::operator=(string&& source) -> type& {
  if(&source == this) return *this;
  reset();
  memory::copy(this, &source, sizeof(string));
  source._data = nullptr;
  source._capacity = SSO - 1;
  source._size = 0;
  return *this;
}

//SSO -> COW
inline auto string::_allocate() -> void {
  char _temp[SSO];
  memory::copy(_temp, _text, SSO);
  _data = memory::allocate<char>(_capacity + 1 + sizeof(uint));
  memory::copy(_data, _temp, SSO);
  _refs = (uint*)(_data + _capacity + 1);  //always aligned by 32 via reserve()
  *_refs = 1;
}

//COW -> Unique
inline auto string::_copy() -> void {
  auto _temp = memory::allocate<char>(_capacity + 1 + sizeof(uint));
  memory::copy(_temp, _data, _size = std::min(_capacity, _size));
  _temp[_size] = 0;
  --*_refs;
  _data = _temp;
  _refs = (uint*)(_data + _capacity + 1);
  *_refs = 1;
}

//COW -> Resize
inline auto string::_resize() -> void {
  _data = memory::resize<char>(_data, _capacity + 1 + sizeof(uint));
  _refs = (uint*)(_data + _capacity + 1);
  *_refs = 1;
}

inline auto string::operator[](uint position) const -> const char& {
  #ifdef DEBUG
  struct out_of_bounds {};
  if(position >= size() + 1) throw out_of_bounds{};
  #endif
  return data()[position];
}

inline auto string::operator()(uint position, char fallback) const -> char {
  if(position >= size() + 1) return fallback;
  return data()[position];
}

template<typename... P> inline auto string::assign(P&&... p) -> string& {
  resize(0);
  return append(forward<P>(p)...);
}

template<typename T, typename... P> inline auto string::prepend(const T& value, P&&... p) -> string& {
  if constexpr(sizeof...(p)) prepend(forward<P>(p)...);
  return _prepend(make_string(value));
}

template<typename... P> inline auto string::prepend(const nall::string_format& value, P&&... p) -> string& {
  if constexpr(sizeof...(p)) prepend(forward<P>(p)...);
  return format(value);
}

template<typename T> inline auto string::_prepend(const stringify<T>& source) -> string& {
  resize(source.size() + size());
  memory::move(get() + source.size(), get(), size() - source.size());
  memory::copy(get(), source.data(), source.size());
  return *this;
}

template<typename T, typename... P> inline auto string::append(const T& value, P&&... p) -> string& {
  _append(make_string(value));
  if constexpr(sizeof...(p) > 0) append(forward<P>(p)...);
  return *this;
}

template<typename... P> inline auto string::append(const nall::string_format& value, P&&... p) -> string& {
  format(value);
  if constexpr(sizeof...(p)) append(forward<P>(p)...);
  return *this;
}

template<typename T> inline auto string::_append(const stringify<T>& source) -> string& {
  resize(size() + source.size());
  memory::copy(get() + size() - source.size(), source.data(), source.size());
  return *this;
}

inline auto string::length() const -> uint {
  return strlen(data());
}

}
