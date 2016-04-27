#pragma once

#include <unordered_set>

#include "Common.h"
#include "IR.h"
#include "Module.h"

namespace nir {

class ConstProp {
  public:
  enum AbsDomain {
    AbsUnknown, AbsOne, AbsMany
  };

  struct AbsValue {
    AbsDomain state;
    Value *value = nullptr;

    // Used for blocks
    bool is_queued = false;
    std::unordered_set<FnCallValue*> callers;

    AbsValue() { }
    AbsValue(const AbsValue&) = delete;

    bool meet(AbsValue& other);
  };

  private:

  Module& module;

  std::vector<FnValue*> worklist;
  std::vector<P<AbsValue>> absvalues;

  bool meet(AbsValue& target, AbsValue& other);

  // TODO: better name
  void markDynamic(FnValue& fn);

  void queue(FnValue&);
  void analyze(Block&);
  void analyze();
  void propagate();
  void propagate(Block& block);

  AbsValue& getAbs(Node& node) {
    auto abs = module.getExtra<AbsValue>(node);
    if (!*abs) {
      auto absnew = std::make_unique<AbsValue>();
      *abs = absnew.get();
      absvalues.push_back(std::move(absnew));
    }
    return **abs;
  }

  public:

  ConstProp(Module& module) : module(module) { }

  void run();
};
}

