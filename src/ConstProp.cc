#include "ConstProp.h"

using namespace nir;

using AbsValue = ConstProp::AbsValue;

bool AbsValue::meet(AbsValue& other) {
  if (other.state == AbsUnknown) {
    return false;
  }

  if (state == AbsUnknown) {
    state = other.state;
    value = other.value;
    return true;
  } else if (state == AbsOne) {
    if (other.state == 1) {
      if (value->eq(*other.value)) {
        return false;
      }
    }

    state = AbsMany;
    return true;
  } else {
    return false;
  }
}

void ConstProp::run() {
  module.resetPass();
  analyze();
  propagate();
}

bool ConstProp::meet(AbsValue& target, AbsValue& other) {
  FnValue* fn1 = nullptr;
  FnValue* fn2 = nullptr;
  if (target.state == AbsOne) {
    fn1 = dynamic_cast<FnValue*>(target.value);
  }
  if (other.state == AbsOne) {
    fn2 = dynamic_cast<FnValue*>(other.value);
  }

  bool res = target.meet(other);
  if (res && target.state == AbsMany) {
    if (fn1) markDynamic(*fn1);
    if (fn2) markDynamic(*fn2);
  }
  return res;
}

void ConstProp::markDynamic(FnValue& fn) {
  AbsValue many;
  many.state = AbsMany;

  for (auto &param : fn.params) {
    if (meet(getAbs(param), many)) {
      for (auto uses : param.users) {
        queue(*uses->owner);
      }
    }
  }
}

void ConstProp::analyze() {
  for (auto &fn : module.fns) {
    auto &abs = getAbs(*fn);
    abs.state = AbsOne;
    abs.value = &*fn;

    if (fn->isExported) {
      // We must handle all parameters
      markDynamic(*fn);
    }

    queue(*fn);
  }

  while (worklist.size()) {
    auto fn = worklist.back();
    worklist.pop_back();

    auto &abs = getAbs(*fn);
    abs.is_queued = false;
    analyze(fn->body);

    auto &abslast = getAbs(*fn->body.values.back());
    auto &absblock = getAbs(fn->body);
    // Meet the return value
    if (meet(absblock, abslast)) {
      // If it changed, queue all the owners again
      for (auto caller : abs.callers) {
        queue(*caller->owner);
      }
    }
  }
}

void ConstProp::queue(FnValue& fn) {
  auto &abs = getAbs(fn);
  if (fn.body.isEmpty()) {
    return;
  }
  if (!abs.is_queued) {
    worklist.push_back(&fn);
    abs.is_queued = true;
  }
}

void ConstProp::analyze(Block& block) {
  for (auto &value : block.values) {
    auto vptr = value.get();
    auto &abs = getAbs(*value);

    if (auto vint = dynamic_cast<IntValue*>(vptr)) {
      abs.state = AbsOne;
      abs.value = vint;
      continue;
    }
    
    if (auto vcopy = dynamic_cast<CopyValue*>(vptr)) {
      meet(abs, getAbs(vcopy->getOriginal()));
      continue;
    }

    if (auto vfn = dynamic_cast<FnValue*>(vptr)) {
      abs.state = AbsOne;
      abs.value = vfn;
      continue;
    }
    
    if (auto vfncall = dynamic_cast<FnCallValue*>(vptr)) {
      auto &fnabs = getAbs(vfncall->getFn());
      auto fn = dynamic_cast<FnValue*>(fnabs.value);

      if (fnabs.state == AbsUnknown) {
        continue;
      }
      
      if (fnabs.state == AbsMany || !fn->isComplete()) {
        struct AbsValue many;
        many.state = AbsMany;
        meet(abs, many);
        continue;
      }

      getAbs(*fn).callers.insert(vfncall);

      auto &absblock = getAbs(fn->body);
      // Meet the return value
      meet(abs, absblock);

      int i = 0;
      auto arg = vfncall->begin_args();
      auto argend = vfncall->end_args();
      for (; arg != argend; ++arg) {
        auto &param = fn->params[i];
        if (meet(getAbs(param), getAbs(**arg))) {
          for (auto uses : param.users) {
            queue(*uses->owner);
          }
        }
        i++;
      }

      if (absblock.state == AbsUnknown) {
        // make sure we queue it at least once
        queue(*fn);
      }

      continue;
    }
    
    if (auto vbr = dynamic_cast<BranchValue*>(vptr)) {
      auto &abscond = getAbs(vbr->getCond());
      IntValue* valcond;

      switch (abscond.state) {
        case AbsUnknown:
          // dead code
          break;
        case AbsOne:
          valcond = dynamic_cast<IntValue*>(abscond.value);
          assert(valcond);
          if (valcond->value != 0) {
            analyze(vbr->tbranch);
            meet(abs, getAbs(*vbr->tbranch.values.back()));
          } else {
            analyze(vbr->fbranch);
            meet(abs, getAbs(*vbr->fbranch.values.back()));
          }
          break;
        case AbsMany:
          analyze(vbr->tbranch);
          meet(abs, getAbs(*vbr->tbranch.values.back()));

          analyze(vbr->fbranch);
          meet(abs, getAbs(*vbr->fbranch.values.back()));
          break;
      }
      continue;
    }

    assert(0 && "unknown value");
  }
}

void ConstProp::propagate() {
  for (auto &fn : module.fns) {
    propagate(fn->body);
  }
}

void ConstProp::propagate(Block& block) {
  for (auto &value : block.values) {
    auto &abs = getAbs(*value);
    if (abs.state == AbsOne && abs.value != value.get()) {
      if (auto cons = dynamic_cast<Constant*>(abs.value)) {
        value.reset(cons->copy().release());
        value->addedTo(block);
      }
    }
  }
}

