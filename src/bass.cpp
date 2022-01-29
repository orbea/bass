//bass
//license: ISC
//project started: 2013-09-27

#include <nall/intrinsics.hpp>
#include <nall/memory.hpp>
#include <nall/iterator.hpp>
#include <nall/range.hpp>
#include <nall/array-view.hpp>
#include <nall/array-span.hpp>
#include <nall/array.hpp>
#include <nall/atoi.hpp>
#include <nall/bit.hpp>
#include <nall/primitives.hpp>
#include <nall/function.hpp>
#include <nall/maybe.hpp>
#include <nall/merge-sort.hpp>
#include <nall/vector.hpp>
#include <nall/shared-pointer.hpp>
#include <nall/string.hpp>
#include <nall/location.hpp>
#include <nall/inode.hpp>
#include <nall/hash.hpp>
#include <nall/file-buffer.hpp>
#include <nall/file.hpp>
#include <nall/directory.hpp>
#include <nall/path.hpp>
#include <nall/hashset.hpp>
#include <nall/set.hpp>
#include <nall/terminal.hpp>

#include "bass.hpp"
#include "core/core.cpp"
#include "architecture/table/table.cpp"

#include <nall/arguments.hpp>
#include <nall/main.hpp>

void nall::main(Arguments arguments) {
  if(!arguments) {
    nall::print(stderr, "bass v18\n");
    nall::print(stderr, "\n");
    nall::print(stderr, "usage:\n");
    nall::print(stderr, "  bass [options] source [source ...]\n");
    nall::print(stderr, "\n");
    nall::print(stderr, "options:\n");
    nall::print(stderr, "  -o target        specify default output filename [overwrite]\n");
    nall::print(stderr, "  -m target        specify default output filename [modify]\n");
    nall::print(stderr, "  -d name[=value]  create define with optional value\n");
    nall::print(stderr, "  -c name[=value]  create constant with optional value\n");
    nall::print(stderr, "  -strict          upgrade warnings to errors\n");
    nall::print(stderr, "  -benchmark       benchmark performance\n");
    exit(EXIT_FAILURE);
  }

  nall::string targetFilename;
  bool create = false;
  if(arguments.take("-o", targetFilename)) create = true;
  if(arguments.take("-m", targetFilename)) create = false;

  nall::vector<nall::string> defines;
  nall::string define;
  while(arguments.take("-d", define)) defines.append(define);

  nall::vector<nall::string> constants;
  nall::string constant;
  while(arguments.take("-c", constant)) constants.append(constant);

  bool strict = arguments.take("-strict");
  bool benchmark = arguments.take("-benchmark");

  if(arguments.find("-*")) {
    nall::print(stderr, "error: unrecognized argument(s)\n");
    exit(EXIT_FAILURE);
  }

  nall::vector<nall::string> sourceFilenames;
  for(auto& argument : arguments) sourceFilenames.append(argument);

  clock_t clockStart = clock();
  Bass bass;
  bass.target(targetFilename, create);
  for(auto& sourceFilename : sourceFilenames) {
    bass.source(sourceFilename);
  }
  for(auto& define : defines) {
    auto p = define.split("=", 1L);
    bass.define(p(0), p(1));
  }
  for(auto& constant : constants) {
    auto p = constant.split("=", 1L);
    bass.constant(p(0), p(1, "1"));
  }
  if(!bass.assemble(strict)) {
    nall::print(stderr, "bass: assembly failed\n");
    exit(EXIT_FAILURE);
  }
  clock_t clockFinish = clock();
  if(benchmark) {
    nall::print(stderr, "bass: assembled in ", (double)(clockFinish - clockStart) / CLOCKS_PER_SEC, " seconds\n");
  }
}
