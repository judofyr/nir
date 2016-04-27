#include "IRPrinter.h"

using namespace nir;

void IRPrinter::print(Module& mod) {
  module = &mod;
  module->resetPass();

  Scope scope(*this);

  for (auto &fn : module->fns) {
    assignName(*fn, scope);
    print(*fn, scope);
  }

  module = nullptr;
}

void IRPrinter::print(Block& block, Scope& scope) {
  for (auto &value : block.values) {
    assignName(*value, scope);
    print(*value, scope);
  }
}

void IRPrinter::assignName(Value& value, Scope& scope) {
  if (value.name.size() == 0) {
    auto storedName = (int*)module->getExtra<void>(value);
    *storedName = scope.acquireName();
  }
}

void IRPrinter::printName(Value& value) {
  if (value.name.size()) {
    output << "%" << value.name;
  } else {
    auto name = (int*)module->getExtra<void>(value);
    output << "%" << *name;
  }
}

#define PRINT_PADDING for (int i = 0; i < scope.level; i++) { output << "  "; }

void IRPrinter::print(Value& value, Scope& scope) {
  PRINT_PADDING;

  printName(value);
  output << " = ";

  if (auto vint = dynamic_cast<IntValue*>(&value)) {
    output << vint->value;
  }

  if (auto vdup = dynamic_cast<CopyValue*>(&value)) {
    printName(vdup->getOriginal());
  }

  if (auto vfn = dynamic_cast<FnValue*>(&value)) {
    auto sub = scope.indent();
    output << "(";
    int i = 0;
    for (auto &param : vfn->params) {
      if (i++ > 0) {
        output << ", ";
      }
      assignName(param, sub);
      printName(param);
    }
    output << ") ";

    if (!vfn->body.isEmpty()) {
      output << "{\n";
      print(vfn->body, sub);
      PRINT_PADDING;
      output << "}";
    }
  }

  if (auto vfncall = dynamic_cast<FnCallValue*>(&value)) {
    printName(vfncall->getFn());
    output << "(";
    int i = 0;
    auto arg = vfncall->begin_args();
    auto argend = vfncall->end_args();
    for (; arg != argend; ++arg) {
      if (i++ > 0) {
        output << ", ";
      }
      printName(**arg);
    }
    output << ")";
  }

  if (auto vbr = dynamic_cast<BranchValue*>(&value)) {
    output << "if ";
    printName(vbr->getCond());
    output << " {\n";
    {
      auto sub = scope.indent();
      print(vbr->tbranch, sub);
    }
    PRINT_PADDING;
    output << "} else {\n";
    {
      auto sub = scope.indent();
      print(vbr->fbranch, sub);
    }
    PRINT_PADDING;
    output << "}";
  }

  output << "\n";
}

