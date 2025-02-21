#pragma once

struct Architecture {
  Architecture(Bass& self) : self(self) {
  }

  virtual ~Architecture() {
  }

  virtual bool assemble(const nall::string& statement) {
    return false;
  }

  unsigned pc() const {
    return self.pc();
  }

  Bass::Endian endian() const {
    return self.endian;
  }

  void setEndian(Bass::Endian endian) {
    self.endian = endian;
  }

  Bass::Directives& directives() {
    return self.directives;
  }

  nall::string readArchitecture(const nall::string& s) {
    return self.readArchitecture(s);
  }

  int64_t evaluate(const nall::string& expression, Bass::Evaluation mode = Bass::Evaluation::Default) {
    return self.evaluate(expression, mode);
  }

  void write(uint64_t data, unsigned length = 1) {
    return self.write(data, length);
  }

  template<typename... P> void notice(P&&... p) {
    return self.notice(std::forward<P>(p)...);
  }

  template<typename... P> void warning(P&&... p) {
    return self.warning(std::forward<P>(p)...);
  }

  template<typename... P> void error(P&&... p) {
    return self.error(std::forward<P>(p)...);
  }

  Bass& self;
};
