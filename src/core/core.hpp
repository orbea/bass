#pragma once

struct Architecture;

struct Bass {
  bool target(const nall::string& filename, bool create);
  bool source(const nall::string& filename);
  void define(const nall::string& name, const nall::string& value);
  void constant(const nall::string& name, const nall::string& value);
  bool assemble(bool strict = false);

  enum class Phase : unsigned { Analyze, Query, Write };
  enum class Endian : unsigned { LSB, MSB };
  enum class Evaluation : unsigned { Default = 0, Strict = 1 };  //strict mode disallows forward-declaration of constants

  struct Instruction {
    nall::string statement;
    unsigned ip;

    unsigned fileNumber;
    unsigned lineNumber;
    unsigned blockNumber;
  };

  struct Macro {
    Macro() {}
    Macro(const nall::string& name) : name(name) {}
    Macro(const nall::string& name, const nall::vector<nall::string>& parameters, unsigned ip, bool inlined) : name(name), parameters(parameters), ip(ip), inlined(inlined) {}

    unsigned hash() const { return name.hash(); }
    bool operator==(const Macro& source) const { return name == source.name; }
    bool operator< (const Macro& source) const { return name <  source.name; }

    nall::string name;
    nall::vector<nall::string> parameters;
    unsigned ip;
    bool inlined;
  };

  struct Define {
    Define() {}
    Define(const nall::string& name) : name(name) {}
    Define(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value) : name(name), parameters(parameters), value(value) {}

    unsigned hash() const { return name.hash(); }
    bool operator==(const Define& source) const { return name == source.name; }
    bool operator< (const Define& source) const { return name <  source.name; }

    nall::string name;
    nall::vector<nall::string> parameters;
    nall::string value;
  };

  using Expression = Define;  //Define and Expression structures are identical

  struct Variable {
    Variable() {}
    Variable(const nall::string& name) : name(name) {}
    Variable(const nall::string& name, int64_t value) : name(name), value(value) {}

    unsigned hash() const { return name.hash(); }
    bool operator==(const Variable& source) const { return name == source.name; }
    bool operator< (const Variable& source) const { return name <  source.name; }

    nall::string name;
    int64_t value;
  };

  using Constant = Variable;  //Variable and Constant structures are identical

  struct Array {
    Array() {}
    Array(const nall::string& name) : name(name) {}
    Array(const nall::string& name, nall::vector<int64_t> values) : name(name), values(values) {}

    unsigned hash() const { return name.hash(); }
    bool operator==(const Array& source) const { return name == source.name; }
    bool operator< (const Array& source) const { return name <  source.name; }

    nall::string name;
    nall::vector<int64_t> values;
  };

  struct Frame {
    enum class Level : unsigned {
      Inline,  //use deepest frame (eg for parameters)
      Active,  //use deepest non-inline frame
      Parent,  //use second-deepest non-inline frame
      Global,  //use root frame
    };

    unsigned ip;
    bool inlined;

    nall::hashset<Macro> macros;
    nall::hashset<Define> defines;
    nall::hashset<Expression> expressions;
    nall::hashset<Variable> variables;
    nall::hashset<Array> arrays;
  };

  struct Block {
    unsigned ip;
    nall::string type;
  };

  struct Tracker {
    bool enable = false;
    std::set<int64_t> addresses;
  };

  struct Directives {
  private:
    struct _EmitBytesOp {
      nall::string token;
      unsigned dataLength;
    };

  public:
    nall::vector<_EmitBytesOp> EmitBytes;

    Directives()
    : EmitBytes ({ {"db ", 1}, {"dw ", 2}, {"dl ", 3}, {"dd ", 4}, {"dq ", 8}})
    {}
    
    void add(nall::string token, unsigned dataLength) {
      EmitBytes.append( {token, dataLength} );
    }
  };

protected:
  bool analyzePhase() const { return phase == Phase::Analyze; }
  bool queryPhase() const { return phase == Phase::Query; }
  bool writePhase() const { return phase == Phase::Write; }

  //core.cpp
  unsigned pc() const;
  void seek(unsigned offset);
  void track(unsigned length);
  void write(uint64_t data, unsigned length = 1);

  void printInstruction();
  void printInstructionStack();
  template<typename... P> void notice(P&&... p);
  template<typename... P> void warning(P&&... p);
  template<typename... P> void error(P&&... p);

  //evaluate.cpp
  int64_t evaluate(const nall::string& expression, Evaluation mode = Evaluation::Default);
  int64_t evaluate(nall::Eval::Node* node, Evaluation mode);
  int64_t quantifyParameters(nall::Eval::Node* node);
  nall::vector<int64_t> evaluateParameters(nall::Eval::Node* node, Evaluation mode);
  int64_t evaluateExpression(nall::Eval::Node* node, Evaluation mode);
  nall::string evaluateString(nall::Eval::Node* node);
  int64_t evaluateLiteral(nall::Eval::Node* node, Evaluation mode);
  int64_t evaluateSubscript(nall::Eval::Node* node, Evaluation mode);
  int64_t evaluateAssign(nall::Eval::Node* node, Evaluation mode);

  //analyze.cpp
  bool analyze();
  bool analyzeInstruction(Instruction& instruction);

  //execute.cpp
  bool execute();
  bool executeInstruction(Instruction& instruction);

  //assemble.cpp
  void initialize();
  bool assemble(const nall::string& statement);
  nall::string assembleString(const nall::string& parameters);

  //utility.cpp
  void setMacro(const nall::string& name, const nall::vector<nall::string>& parameters, unsigned ip, bool inlined, Frame::Level level);
  nall::maybe<Bass::Macro&> findMacro(const nall::string& name);

  void setDefine(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value, Frame::Level level);
  nall::maybe<Bass::Define&> findDefine(const nall::string& name);

  void setExpression(const nall::string& name, const nall::vector<nall::string>& parameters, const nall::string& value, Frame::Level level);
  nall::maybe<Bass::Expression&> findExpression(const nall::string& name);

  void setVariable(const nall::string& name, int64_t value, Frame::Level level);
  nall::maybe<Bass::Variable&> findVariable(const nall::string& name);

  void setConstant(const nall::string& name, int64_t value);
  nall::maybe<Bass::Constant&> findConstant(const nall::string& name);

  void setArray(const nall::string& name, const nall::vector<int64_t>& values, Frame::Level level);
  nall::maybe<Bass::Array&> findArray(const nall::string& name);

  void evaluateDefines(nall::string& statement);

  nall::string readArchitecture(const nall::string& s);

  nall::string filepath();
  nall::vector<nall::string> split(const nall::string& s);
  void strip(nall::string& s);
  bool validate(const nall::string& s);
  nall::string text(nall::string s);
  int64_t character(const nall::string& s);

  //internal state
  Instruction* activeInstruction = nullptr;  //used by notice, warning, error
  nall::vector<Instruction> program;    //parsed source code statements
  nall::vector<Block> blocks;           //track the start and end of blocks
  std::set<Define> defines;             //defines specified on the terminal
  nall::hashset<Constant> constants;    //constants support forward-declaration
  nall::vector<Frame> frames;           //macros, defines and variables do not
  nall::vector<bool> conditionals;      //track conditional matching
  nall::vector<nall::string> queue;            //track enqueue, dequeue directives
  nall::vector<nall::string> scope;            //track scope recursion
  int64_t stringTable[256];       //overrides for d[bwldq] text strings
  Phase phase;                    //phase of assembly
  Endian endian = Endian::LSB;    //used for multi-byte writes (d[bwldq], etc)
  Tracker tracker;                //used to track writes to detect overwrites
  unsigned macroInvocationCounter;    //used for {#} support
  unsigned ip = 0;                    //instruction pointer into program
  unsigned origin = 0;                //file offset
  int base = 0;                   //file offset to memory map displacement
  unsigned lastLabelCounter = 1;      //- instance counter
  unsigned nextLabelCounter = 1;      //+ instance counter
  bool charactersUseMap = false;  //0 = '*' parses as ASCII; 1 = '*' uses stringTable[]
  bool strict = false;            //upgrade warnings to errors when true
  Directives directives;          //active directives

  nall::file_buffer targetFile;
  nall::vector<nall::string> sourceFilenames;

  nall::shared_pointer<Architecture> architecture;
  friend class Architecture;
};
