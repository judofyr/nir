#pragma once

#include "Common.h"
#include "Module.h"
#include "IR.h"

#include <ostream>
#include <set>

namespace nir {

class IRPrinter {
  std::ostream& output;

  std::vector<int> freeNames;
  int maxName = 0;
  Module* module = nullptr;

  struct Scope {
    IRPrinter& printer;
    int level = 0;
    std::set<int> names;

    Scope(IRPrinter& printer) : printer(printer) { }

    Scope indent() {
      Scope res(printer);
      res.level = level + 1;
      return std::move(res);
    }

    int acquireName() {
      int name;
      if (printer.freeNames.size()) {
        name = printer.freeNames.back();
        printer.freeNames.pop_back();
      } else {
        name = printer.maxName++;
      }
      names.insert(name);
      return name;
    }

    ~Scope() {
      for (int name : names) {
        printer.freeNames.push_back(name);
      }
    }
  };

  void print(Block&, Scope&);
  void print(Value&, Scope&);
  void printName(Value&);
  void assignName(Value&, Scope&);

  public:

  IRPrinter(std::ostream& output) : output(output) { }

  void print(Module&);
};

}

