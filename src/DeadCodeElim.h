#pragma once

#include "Common.h"
#include "IR.h"
#include "Module.h"

namespace nir {

class DeadCodeElim {
  Module& module;

  public:
  DeadCodeElim(Module& module) : module(module) { }
  void run();
};

}

