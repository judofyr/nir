#pragma once

#include "Common.h"
#include "Type.h"

namespace nir {

struct Context {
  P<VoidType> tyVoid;
  Dict<int, P<IntType>> tyInts;
  P<SingleType> tySingle;
  P<DoubleType> tyDouble;

  VoidType& getTyVoid() { return *tyVoid; }
  IntType& getTyInt(int bitsize) { return *tyInts[bitsize]; }
  SingleType& getTySingle() { return *tySingle; }
  DoubleType& getTyDouble() { return *tyDouble; }
};

}

