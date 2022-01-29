#pragma once

struct Table : Architecture {
  Table(Bass& self, const nall::string& table);
  bool assemble(const nall::string& statement) override;

private:
  struct Prefix {
    nall::string text;
    uint size;
  };

  struct Number {
    uint bits;
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
    enum class Match : uint { Exact, Strong, Weak } match;
    uint data;
    uint bits;
    uint argument;
    int displacement;
  };

  struct Opcode {
    nall::vector<Prefix> prefix;
    nall::vector<Number> number;
    nall::vector<Format> format;
    nall::string pattern;
  };

  uint bitLength(nall::string& text) const;
  void writeBits(uint64_t data, uint bits);
  bool parseTable(const nall::string& text);
  void parseDirective(nall::string& line);
  void assembleTableLHS(Opcode& opcode, const nall::string& text);
  void assembleTableRHS(Opcode& opcode, const nall::string& text);
  uint64_t swapEndian(uint64_t data, unsigned bits);

  nall::vector<Opcode> table;
  uint64_t bitval, bitpos;
};
