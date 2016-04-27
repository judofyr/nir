#pragma once

#include "IR.h"

namespace nir {

struct Module {
  int passLevel = 0;
  std::vector<P<TopFnValue>> fns;

  Module() {
  }

  void resetPass() {
    passLevel += 1;
  }

  TopFnValue& insertFn(FnType& type) {
    auto fn = std::make_unique<TopFnValue>(type);
    auto &res = *fn;
    res.body.owner = &res;
    fns.push_back(std::move(fn));
    return res;
  }

  template<class T>
  T** getExtra(Node& node) {
    if (node.passLevel != passLevel) {
      node.extra = nullptr;
      node.passLevel = passLevel;
    }
    return (T**)&node.extra;
  }
};

}
