#pragma once

struct Table : Architecture {
  Table(Bass& self, const nall::string& table);
  auto assemble(const nall::string& statement) -> bool override;

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

  auto bitLength(nall::string& text) const -> uint;
  auto writeBits(uint64_t data, uint bits) -> void;
  auto parseTable(const nall::string& text) -> bool;
  auto parseDirective(nall::string& line) -> void;
  auto assembleTableLHS(Opcode& opcode, const nall::string& text) -> void;
  auto assembleTableRHS(Opcode& opcode, const nall::string& text) -> void;
  auto swapEndian(uint64_t data, unsigned bits) -> uint64_t;

  nall::vector<Opcode> table;
  uint64_t bitval, bitpos;
};
