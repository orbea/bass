#pragma once

struct Architecture;

struct Bass {
  auto target(const nall::string& filename, bool create) -> bool;
  auto source(const nall::string& filename) -> bool;
  auto define(const nall::string& name, const nall::string& value) -> void;
  auto constant(const nall::string& name, const nall::string& value) -> void;
  auto assemble(bool strict = false) -> bool;

  enum class Phase : uint { Analyze, Query, Write };
  enum class Endian : uint { LSB, MSB };
  enum class Evaluation : uint { Default = 0, Strict = 1 };  //strict mode disallows forward-declaration of constants

  struct Instruction {
    nall::string statement;
    uint ip;

    uint fileNumber;
    uint lineNumber;
    uint blockNumber;
  };

  struct Macro {
    Macro() {}
    Macro(const nall::string& name) : name(name) {}
    Macro(const nall::string& name, const nall::vector<nall::string>& parameters, uint ip, bool inlined) : name(name), parameters(parameters), ip(ip), inlined(inlined) {}

    auto hash() const -> uint { return name.hash(); }
    auto operator==(const Macro& source) const -> bool { return name == source.name; }
    auto operator< (const Macro& source) const -> bool { return name <  source.name; }

    nall::string name;
    nall::vector<nall::string> parameters;
    uint ip;
    bool inlined;
  };

  struct Define {
    Define() {}
    Define(const nall::string& name) : name(name) {}
    Define(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value) : name(name), parameters(parameters), value(value) {}

    auto hash() const -> uint { return name.hash(); }
    auto operator==(const Define& source) const -> bool { return name == source.name; }
    auto operator< (const Define& source) const -> bool { return name <  source.name; }

    nall::string name;
    nall::vector<nall::string> parameters;
    nall::string value;
  };

  using Expression = Define;  //Define and Expression structures are identical

  struct Variable {
    Variable() {}
    Variable(const nall::string& name) : name(name) {}
    Variable(const nall::string& name, int64_t value) : name(name), value(value) {}

    auto hash() const -> uint { return name.hash(); }
    auto operator==(const Variable& source) const -> bool { return name == source.name; }
    auto operator< (const Variable& source) const -> bool { return name <  source.name; }

    nall::string name;
    int64_t value;
  };

  using Constant = Variable;  //Variable and Constant structures are identical

  struct Array {
    Array() {}
    Array(const nall::string& name) : name(name) {}
    Array(const nall::string& name, nall::vector<int64_t> values) : name(name), values(values) {}

    auto hash() const -> uint { return name.hash(); }
    auto operator==(const Array& source) const -> bool { return name == source.name; }
    auto operator< (const Array& source) const -> bool { return name <  source.name; }

    nall::string name;
    nall::vector<int64_t> values;
  };

  struct Frame {
    enum class Level : uint {
      Inline,  //use deepest frame (eg for parameters)
      Active,  //use deepest non-inline frame
      Parent,  //use second-deepest non-inline frame
      Global,  //use root frame
    };

    uint ip;
    bool inlined;

    nall::hashset<Macro> macros;
    nall::hashset<Define> defines;
    nall::hashset<Expression> expressions;
    nall::hashset<Variable> variables;
    nall::hashset<Array> arrays;
  };

  struct Block {
    uint ip;
    nall::string type;
  };

  struct Tracker {
    bool enable = false;
    nall::set<int64_t> addresses;
  };

  struct Directives {
  private:
    struct _EmitBytesOp {
      nall::string token;
      uint dataLength;
    };

  public:
    nall::vector<_EmitBytesOp> EmitBytes;

    Directives()
    : EmitBytes ({ {"db ", 1}, {"dw ", 2}, {"dl ", 3}, {"dd ", 4}, {"dq ", 8}})
    {}
    
    void add(nall::string token, uint dataLength) {
      EmitBytes.append( {token, dataLength} );
    }
  };

protected:
  auto analyzePhase() const -> bool { return phase == Phase::Analyze; }
  auto queryPhase() const -> bool { return phase == Phase::Query; }
  auto writePhase() const -> bool { return phase == Phase::Write; }

  //core.cpp
  auto pc() const -> uint;
  auto seek(uint offset) -> void;
  auto track(uint length) -> void;
  auto write(uint64_t data, uint length = 1) -> void;

  auto printInstruction() -> void;
  auto printInstructionStack() -> void;
  template<typename... P> auto notice(P&&... p) -> void;
  template<typename... P> auto warning(P&&... p) -> void;
  template<typename... P> auto error(P&&... p) -> void;

  //evaluate.cpp
  auto evaluate(const nall::string& expression, Evaluation mode = Evaluation::Default) -> int64_t;
  auto evaluate(nall::Eval::Node* node, Evaluation mode) -> int64_t;
  auto quantifyParameters(nall::Eval::Node* node) -> int64_t;
  auto evaluateParameters(nall::Eval::Node* node, Evaluation mode) -> nall::vector<int64_t>;
  auto evaluateExpression(nall::Eval::Node* node, Evaluation mode) -> int64_t;
  auto evaluateString(nall::Eval::Node* node) -> nall::string;
  auto evaluateLiteral(nall::Eval::Node* node, Evaluation mode) -> int64_t;
  auto evaluateSubscript(nall::Eval::Node* node, Evaluation mode) -> int64_t;
  auto evaluateAssign(nall::Eval::Node* node, Evaluation mode) -> int64_t;

  //analyze.cpp
  auto analyze() -> bool;
  auto analyzeInstruction(Instruction& instruction) -> bool;

  //execute.cpp
  auto execute() -> bool;
  auto executeInstruction(Instruction& instruction) -> bool;

  //assemble.cpp
  auto initialize() -> void;
  auto assemble(const nall::string& statement) -> bool;
  auto assembleString(const nall::string& parameters) -> nall::string;

  //utility.cpp
  auto setMacro(const nall::string& name, const nall::vector<nall::string>& parameters, uint ip, bool inlined, Frame::Level level) -> void;
  auto findMacro(const nall::string& name) -> nall::maybe<Macro&>;

  auto setDefine(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value, Frame::Level level) -> void;
  auto findDefine(const nall::string& name) -> nall::maybe<Define&>;

  auto setExpression(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value, Frame::Level level) -> void;
  auto findExpression(const nall::string& name) -> nall::maybe<Expression&>;

  auto setVariable(const nall::string& name, int64_t value, Frame::Level level) -> void;
  auto findVariable(const nall::string& name) -> nall::maybe<Variable&>;

  auto setConstant(const nall::string& name, int64_t value) -> void;
  auto findConstant(const nall::string& name) -> nall::maybe<Constant&>;

  auto setArray(const nall::string& name, const nall::vector<int64_t>& values, Frame::Level level) -> void;
  auto findArray(const nall::string& name) -> nall::maybe<Array&>;

  auto evaluateDefines(nall::string& statement) -> void;

  auto readArchitecture(const nall::string& s) -> nall::string;

  auto filepath() -> nall::string;
  auto split(const nall::string& s) -> nall::vector<nall::string>;
  auto strip(nall::string& s) -> void;
  auto validate(const nall::string& s) -> bool;
  auto text(nall::string s) -> nall::string;
  auto character(const nall::string& s) -> int64_t;

  //internal state
  Instruction* activeInstruction = nullptr;  //used by notice, warning, error
  nall::vector<Instruction> program;    //parsed source code statements
  nall::vector<Block> blocks;           //track the start and end of blocks
  nall::set<Define> defines;            //defines specified on the terminal
  nall::hashset<Constant> constants;    //constants support forward-declaration
  nall::vector<Frame> frames;           //macros, defines and variables do not
  nall::vector<bool> conditionals;      //track conditional matching
  nall::vector<nall::string> queue;            //track enqueue, dequeue directives
  nall::vector<nall::string> scope;            //track scope recursion
  int64_t stringTable[256];       //overrides for d[bwldq] text strings
  Phase phase;                    //phase of assembly
  Endian endian = Endian::LSB;    //used for multi-byte writes (d[bwldq], etc)
  Tracker tracker;                //used to track writes to detect overwrites
  uint macroInvocationCounter;    //used for {#} support
  uint ip = 0;                    //instruction pointer into program
  uint origin = 0;                //file offset
  int base = 0;                   //file offset to memory map displacement
  uint lastLabelCounter = 1;      //- instance counter
  uint nextLabelCounter = 1;      //+ instance counter
  bool charactersUseMap = false;  //0 = '*' parses as ASCII; 1 = '*' uses stringTable[]
  bool strict = false;            //upgrade warnings to errors when true
  Directives directives;          //active directives

  nall::file_buffer targetFile;
  nall::vector<nall::string> sourceFilenames;

  nall::shared_pointer<Architecture> architecture;
  friend class Architecture;
};
