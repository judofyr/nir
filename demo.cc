#include "src/Context.h"
#include "src/Module.h"
#include "src/ConstProp.h"
#include "src/DeadCodeElim.h"
#include "src/IRPrinter.h"

#include <iostream>

using namespace nir;

int main(int argc, char* argv[]) {
  Context c;
  Module mod;
  auto &int32 = c.getTyInt(32);

  Type* params[] = { &int32 };
  auto fntype = std::make_unique<FnType>(int32, params);
  auto &fn = mod.insertFn(*fntype);

  {
    // Helper function
    auto &b = fn.body;
    fn.body.insert<CopyValue>(fn.params[0]);
  }

  {
    // Main function
    auto &main = mod.insertFn(*fntype);
    main.isExported = true;
    auto &b = main.body;

    auto &one = b.insert<IntValue>(int32, 1);
    one.name = "a";

    Value* args[] = { &one };
    auto &res = b.insert<FnCallValue>(fn, args);

    auto &two = b.insert<IntValue>(int32, 2);
    Value* args2[] = { &two };
    b.insert<FnCallValue>(fn, args2);

    auto &br = b.insert<BranchValue>(res);
    br.tbranch.insert<IntValue>(int32, 1);
    br.fbranch.insert<IntValue>(int32, 2);
  }

  ConstProp cp(mod);
  DeadCodeElim dce(mod);

  if (argc > 1) {
    dce.run();
    cp.run();
    dce.run();
  }

  IRPrinter printer(std::cout);
  printer.print(mod);
}

