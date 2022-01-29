#pragma once

struct Table : Architecture {
  Table(Bass& self, const nall::string& table);
  bool assemble(const nall::string& statement) override;

private:
  struct Prefix {
    nall::string text;
    unsigned size;
  };

  struct Number {
    unsigned bits;
  };

  struct Format {
    enum class Type : unsigned {
        Static, Absolute,
        Relative,
        Repeat,
        ShiftRight, ShiftLeft,
        RelativeShiftRight,
        Negative,
        NegativeShiftRight
    } type;
    enum class Match : unsigned { Exact, Strong, Weak } match;
    unsigned data;
    unsigned bits;
    unsigned argument;
    int displacement;
  };

  struct Opcode {
    std::vector<Prefix> prefix;
    std::vector<Number> number;
    std::vector<Format> format;
    nall::string pattern;
  };

  unsigned bitLength(nall::string& text) const;
  void writeBits(uint64_t data, unsigned bits);
  bool parseTable(const nall::string& text);
  void parseDirective(nall::string& line);
  void assembleTableLHS(Opcode& opcode, const nall::string& text);
  void assembleTableRHS(Opcode& opcode, const nall::string& text);
  uint64_t swapEndian(uint64_t data, unsigned bits);

  std::vector<Opcode> table;
  uint64_t bitval, bitpos;
};
