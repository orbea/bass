#pragma once

namespace nall {

struct directory : inode {
  directory() = delete;

  static auto exists(const string& pathname) -> bool;
};

#if defined(PLATFORM_WINDOWS)
  inline auto directory::exists(const string& pathname) -> bool {
    if(!pathname) return false;
    string name = pathname;
    name.trim("\"", "\"");
    DWORD result = GetFileAttributes(utf16_t(name));
    if(result == INVALID_FILE_ATTRIBUTES) return false;
    return (result & FILE_ATTRIBUTE_DIRECTORY);
  }
#else
  inline auto directory::exists(const string& pathname) -> bool {
    if(!pathname) return false;
    struct stat data;
    if(stat(pathname, &data) != 0) return false;
    return S_ISDIR(data.st_mode);
  }
#endif

}
