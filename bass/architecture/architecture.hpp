struct Architecture {
  Architecture(Bass& self) : self(self) {
  }

  virtual ~Architecture() {
  }

  virtual auto assemble(const nall::string& statement) -> bool {
    return false;
  }

  //

  auto pc() const -> uint {
    return self.pc();
  }

  auto endian() const -> Bass::Endian {
    return self.endian;
  }

  auto setEndian(Bass::Endian endian) -> void {
    self.endian = endian;
  }

  auto directives() -> Bass::Directives& {
    return self.directives;
  }

  auto readArchitecture(const nall::string& s) -> nall::string {
    return self.readArchitecture(s);
  }

  auto evaluate(const nall::string& expression, Bass::Evaluation mode = Bass::Evaluation::Default) -> int64_t {
    return self.evaluate(expression, mode);
  }

  auto write(uint64_t data, uint length = 1) -> void {
    return self.write(data, length);
  }

  template<typename... P> auto notice(P&&... p) -> void {
    return self.notice(nall::forward<P>(p)...);
  }

  template<typename... P> auto warning(P&&... p) -> void {
    return self.warning(nall::forward<P>(p)...);
  }

  template<typename... P> auto error(P&&... p) -> void {
    return self.error(nall::forward<P>(p)...);
  }

  Bass& self;
};
