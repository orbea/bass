Table::Table(Bass& self, const nall::string& table) : Architecture(self) {
  bitval = 0;
  bitpos = 0;
  parseTable(table);
}

bool Table::assemble(const nall::string& statement) {
  nall::string s = statement;

  if(s.match("instrument \"*\"")) {
    s.trim("instrument \"", "\"", 1L);
    parseTable(s);
    return true;
  }

  unsigned pc = Architecture::pc();

  for(auto& opcode : table) {
    if(!tokenize(s, opcode.pattern)) continue;

    nall::vector<nall::string> args;
    tokenize(args, s, opcode.pattern);
    if(args.size() != opcode.number.size()) continue;

    bool mismatch = false;
    for(auto& format : opcode.format) {
      if(format.type == Format::Type::Absolute) {
        if(format.match != Format::Match::Weak) {
          unsigned bits = bitLength(args[format.argument]);
          if(bits != opcode.number[format.argument].bits) {
            if(format.match == Format::Match::Exact || bits != 0) {
              mismatch = true;
              break;
            }
          }
        }
      }
    }
    if(mismatch) continue;

    for(auto& format : opcode.format) {
      switch(format.type) {
        case Format::Type::Static: {
          writeBits(format.data, format.bits);
          break;
        }

        case Format::Type::Absolute: {
          unsigned data = evaluate(args[format.argument]);
          writeBits(data, opcode.number[format.argument].bits);
          break;
        }

        case Format::Type::Relative: {
          int data = evaluate(args[format.argument]) - (pc + format.displacement);
          unsigned bits = opcode.number[format.argument].bits;
          int min = -(1 << (bits - 1)), max = +(1 << (bits - 1)) - 1;
          if(data < min || data > max) {
            error("branch out of bounds: ", data);
          }
          writeBits(data, opcode.number[format.argument].bits);
          break;
        }

        case Format::Type::Repeat: {
          unsigned data = evaluate(args[format.argument]);
          for(unsigned n : nall::range(data)) {
            writeBits(format.data, opcode.number[format.argument].bits);
          }
          break;
        }

        case Format::Type::ShiftRight: {
          uint64_t data = evaluate(args[format.argument]);
          writeBits(data >> format.data, opcode.number[format.argument].bits);
          break;
        }

        case Format::Type::ShiftLeft: {
          uint64_t data = evaluate(args[format.argument]);
          writeBits(data << format.data, opcode.number[format.argument].bits);
          break;
        }

        case Format::Type::RelativeShiftRight: {
          int data = evaluate(args[format.argument]) - (pc + format.displacement);
          unsigned bits = opcode.number[format.argument].bits;
          int min = -(1 << (bits - 1)), max = +(1 << (bits - 1)) - 1;
          if(data < min || data > max) error("branch out of bounds");
          bits -= format.data;
          if (endian() == Bass::Endian::LSB) {
            writeBits(data >> format.data, bits);
          } else {
            data >>= format.data;
            writeBits(swapEndian(data, bits), bits);
          }
          break;
        }

        case Format::Type::Negative: {
          unsigned data = evaluate(args[format.argument]);
          writeBits(-data, opcode.number[format.argument].bits);
          break;
        }

        case Format::Type::NegativeShiftRight: {
          uint64_t data = evaluate(args[format.argument]);
          writeBits(-data >> format.data, opcode.number[format.argument].bits);
          break;
        }        
      }
    }

    return true;
  }

  return false;
}

unsigned Table::bitLength(nall::string& text) const {
  auto binLength = [&](const char* p) -> unsigned {
    unsigned length = 0;
    while(*p) {
      if(*p == '0' || *p == '1') { p++; length += 1; continue; }
      return 0;
    }
    return length;
  };

  auto hexLength = [&](const char* p) -> unsigned {
    unsigned length = 0;
    while(*p) {
      if(*p >= '0' && *p <= '9') { p++; length += 4; continue; }
      if(*p >= 'a' && *p <= 'f') { p++; length += 4; continue; }
      if(*p >= 'A' && *p <= 'F') { p++; length += 4; continue; }
      return 0;
    }
    return length;
  };

  char* p = text.get();
  if(*p == '<') { *p = ' '; return  8; }
  if(*p == '>') { *p = ' '; return 16; }
  if(*p == '^') { *p = ' '; return 24; }
  if(*p == '?') { *p = ' '; return 32; }
  if(*p == ':') { *p = ' '; return 64; }
  if(*p == '%') return binLength(p + 1);
  if(*p == '$') return hexLength(p + 1);
  if(*p == '0' && *(p + 1) == 'b') return binLength(p + 2);
  if(*p == '0' && *(p + 1) == 'x') return hexLength(p + 2);
  return 0;
}

void Table::writeBits(uint64_t data, unsigned length) {
  nall::function<uint64_t(unsigned)> setBits = [&](unsigned n) -> uint64_t {
    // Create a bit mask with the n least significant bits set
    return (1 << n) - 1;
  };
  bitval <<= length;
  bitval |= data & setBits(length);
  bitpos += length;

  while(bitpos >= 8) {
    write(bitval);
    bitval >>= 8;
    bitpos -= 8;
  }  
}

bool Table::parseTable(const nall::string& text) {
  auto lines = text.split("\n");
  for(auto& line : lines) {
    if(auto position = line.find("//")) line.resize(position());  //remove comments

    if(line[0] == '#') {
      if(line == "#endian lsb") { setEndian(Bass::Endian::LSB); continue; }
      if(line == "#endian msb") { setEndian(Bass::Endian::MSB); continue; }

      if(auto position = line.find("#include ") ) {
        line.trimLeft("#include ", 1L);
        auto more = readArchitecture(line.strip());
        parseTable(more);
        continue;
      }
      if(auto position = line.find("#directive ") ) {
        parseDirective(line);
      }
    }

    auto part = line.split(";", 1L).strip();
    if(part.size() != 2) continue;

    Opcode opcode;
    assembleTableLHS(opcode, part(0));
    assembleTableRHS(opcode, part(1));
    table.push_back(opcode);
  }

  return true;
}

// #directive <name> <byte_size>
void Table::parseDirective(nall::string& line) {
  auto work = line.strip();
  work.trimLeft("#directive ", 1L);
  
  auto items = work.split(" ");
  if(items.size() != 2) {
    error("Wrong syntax: '",line , "'\n");
  }

  auto& key = items[0];
  key.append(" ");
  
  unsigned value = atoi(items[1]);
  
  for(auto& d : directives().EmitBytes) {
    if(key.equals(d.token)) {
      d.dataLength = value;
      return;
    }
  }
  
  directives().add(key, value);
}


void Table::assembleTableLHS(Opcode& opcode, const nall::string& text) {
  unsigned offset = 0;

  auto length = [&] {
    unsigned length = 0;
    while(text[offset + length]) {
      char n = text[offset + length];
      if(n == '*') break;
      length++;
    }
    return length;
  };

  while(text[offset]) {
    unsigned size = length();
    opcode.prefix.push_back({slice(text, offset, size), size});
    offset += size;

    if(text[offset] != '*') continue;
    unsigned bits = 10 * (text[offset + 1] - '0');
    bits += text[offset + 2] - '0';
    opcode.number.push_back({bits});
    offset += 3;
  }

  for(auto& prefix : opcode.prefix) {
    opcode.pattern.append(prefix.text, "*");
  }
  opcode.pattern.trimRight("*", 1L);
  if(opcode.number.size() == opcode.prefix.size()) opcode.pattern.append("*");
}

void Table::assembleTableRHS(Opcode& opcode, const nall::string& text) {
  unsigned offset = 0;

  auto list = text.split(" ");
  for(auto& item : list) {
    if(item[0] == '$' && item.length() == 3) {
      Format format = {Format::Type::Static};
      format.data = nall::toHex((const char*)item + 1);
      format.bits = (item.length() - 1) * 4;
      opcode.format.push_back(format);
    }

// >>XXa
    else if(item[0] == '>' && item[1] == '>') {
      Format format = {Format::Type::ShiftRight, Format::Match::Weak};
      format.argument = item[4] - 'a';
      format.data = (item[2] - '0') * 10 + (item[3] - '0');
      opcode.format.push_back(format);
    }

    else if(item[0] == '<' && item[1] == '<') {
      Format format = {Format::Type::ShiftLeft, Format::Match::Weak};
      format.argument = item[4] - 'a';
      format.data = (item[2] - '0') * 10 + (item[3] - '0');
      opcode.format.push_back(format);
    }

    // +X>>YYa
    else if(item[0] == '+' && item[2] == '>' && item[3] == '>') {
      Format format = {Format::Type::RelativeShiftRight, Format::Match::Weak};
      format.argument = item[6] - 'a';
      format.displacement = +(item[1] - '0');
      format.data = (item[4] - '0') * 10 + (item[5] - '0');
      opcode.format.push_back(format);
    }

    // N>>XXa
    else if(item[0] == 'N' && item[1] == '>' && item[2] == '>') {
      Format format = {Format::Type::NegativeShiftRight, Format::Match::Weak};
      format.argument = item[5] - 'a';
      format.data = (item[3] - '0') * 10 + (item[4] - '0');
      opcode.format.push_back(format);
    }

    // Na
    else if(item[0] == 'N' && item[1] != '>') {
      Format format = {Format::Type::Negative, Format::Match::Weak};
      format.argument = item[1] - 'a';
      opcode.format.push_back(format);
    }

    else if(item[0] == '%') {
      Format format = {Format::Type::Static};
      format.data = nall::toBinary((const char*)item + 1);
      format.bits = (item.length() - 1);
      opcode.format.push_back(format);
    }

    else if(item[0] == '!') {
      Format format = {Format::Type::Absolute, Format::Match::Exact};
      format.argument = item[1] - 'a';
      opcode.format.push_back(format);
    }

    else if(item[0] == '=') {
      Format format = {Format::Type::Absolute, Format::Match::Strong};
      format.argument = item[1] - 'a';
      opcode.format.push_back(format);
    }

    else if(item[0] == '~') {
      Format format = {Format::Type::Absolute, Format::Match::Weak};
      format.argument = item[1] - 'a';
      opcode.format.push_back(format);
    }

    else if(item[0] == '+') {
      Format format = {Format::Type::Relative};
      format.argument = item[2] - 'a';
      format.displacement = +(item[1] - '0');
      opcode.format.push_back(format);
    }

    else if(item[0] == '-') {
      Format format = {Format::Type::Relative};
      format.argument = item[2] - 'a';
      format.displacement = -(item[1] - '0');
      opcode.format.push_back(format);
    }

    else if(item[0] == '*') {
      Format format = {Format::Type::Repeat};
      format.argument = item[1] - 'a';
      format.data = nall::toHex((const char*)item + 3);
      opcode.format.push_back(format);
    }
  }
}

uint64_t Table::swapEndian(uint64_t data, unsigned bits) {
  int t_data = 0;
  switch((bits - 1) / 8) {
    case 3: { // 4 bytes
      t_data = ((data & 0xFF000000) >> 24) | \
               ((data & 0x00FF0000) >> 8) | \
               ((data & 0x0000FF00) << 8) | \
               ((data & 0x000000FF) << 24);
      break;
    }
    case 2: { // 3 bytes
      t_data = ((data & 0xFF0000) >> 16) | \
               ((data & 0x00FF00)) | \
               ((data & 0x0000FF) << 16);
      break;
    }
    case 1: { // 2 bytes
      t_data = ((data & 0xFF00) >> 8) | \
               ((data & 0x00FF) << 8);
      break;
    }
    case 0: { // byte
      t_data = data;
      break;
    }
    default: {
      error("Invalid number of bits for BassTable::swapEndian");
      break;
    }
  }
  return t_data;
}
