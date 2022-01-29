#pragma once

//generic abstraction layer for common storage operations against both files and directories
//these functions are not recursive; use directory::create() and directory::remove() for recursion

namespace nall {

struct inode {
  enum class time : unsigned { create, modify, access };

  inode() = delete;
  inode(const inode&) = delete;
  auto operator=(const inode&) -> inode& = delete;

  static auto writable(const string& name) -> bool {
    return access(name, W_OK) == 0;
  }

  //returns false if 'name' is a directory that is not empty
  static auto remove(const string& name) -> bool {
    if(name.endsWith("/")) return rmdir(name) == 0;
    return unlink(name) == 0;
  }
};

}
