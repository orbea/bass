#pragma once

struct Architecture {
  Architecture(Bass& self) : self(self) {
  }

  virtual ~Architecture() {
  }

  virtual bool assemble(const nall::string& statement) {
    return false;
  }

  uint pc() const {
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

  void write(uint64_t data, uint length = 1) {
    return self.write(data, length);
  }

  template<typename... P> void notice(P&&... p) {
    return self.notice(nall::forward<P>(p)...);
  }

  template<typename... P> void warning(P&&... p) {
    return self.warning(nall::forward<P>(p)...);
  }

  template<typename... P> void error(P&&... p) {
    return self.error(nall::forward<P>(p)...);
  }

  Bass& self;
};
