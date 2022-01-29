#pragma once

namespace nall {

struct string_pascal {
  using type = string_pascal;

  string_pascal(const char* text = nullptr) {
    if(text && *text) {
      unsigned size = strlen(text);
      _data = memory::allocate<char>(sizeof(unsigned) + size + 1);
      ((unsigned*)_data)[0] = size;
      memory::copy(_data + sizeof(unsigned), text, size);
      _data[sizeof(unsigned) + size] = 0;
    }
  }

  string_pascal(const string& text) {
    if(text.size()) {
      _data = memory::allocate<char>(sizeof(unsigned) + text.size() + 1);
      ((unsigned*)_data)[0] = text.size();
      memory::copy(_data + sizeof(unsigned), text.data(), text.size());
      _data[sizeof(unsigned) + text.size()] = 0;
    }
  }

  string_pascal(const string_pascal& source) { operator=(source); }
  string_pascal(string_pascal&& source) { operator=(std::move(source)); }

  ~string_pascal() {
    if(_data) memory::free(_data);
  }

  explicit operator bool() const { return _data; }
  operator const char*() const { return _data ? _data + sizeof(unsigned) : nullptr; }
  operator string() const { return _data ? string{_data + sizeof(unsigned)} : ""; }

  auto operator=(const string_pascal& source) -> type& {
    if(this == &source) return *this;
    if(_data) { memory::free(_data); _data = nullptr; }
    if(source._data) {
      unsigned size = source.size();
      _data = memory::allocate<char>(sizeof(unsigned) + size);
      memory::copy(_data, source._data, sizeof(unsigned) + size);
    }
    return *this;
  }

  auto operator=(string_pascal&& source) -> type& {
    if(this == &source) return *this;
    if(_data) memory::free(_data);
    _data = source._data;
    source._data = nullptr;
    return *this;
  }

  auto operator==(string_view source) const -> bool {
    return size() == source.size() && memory::compare(data(), source.data(), size()) == 0;
  }

  auto operator!=(string_view source) const -> bool {
    return size() != source.size() || memory::compare(data(), source.data(), size()) != 0;
  }

  auto data() const -> char* {
    if(!_data) return nullptr;
    return _data + sizeof(unsigned);
  }

  auto size() const -> unsigned {
    if(!_data) return 0;
    return ((unsigned*)_data)[0];
  }

protected:
  char* _data = nullptr;
};

}
