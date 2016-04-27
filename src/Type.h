#pragma once

namespace nir {

struct Type {
  virtual ~Type() { }
};

struct VoidType : Type {
};

struct IntType : Type {
  int bitsize;
};

struct FloatType : Type {
};

struct SingleType : FloatType {
};

struct DoubleType : FloatType {
};

struct FnType : Type {
  Type& ret;
  std::vector<Type*> params;

  FnType(Type& ret, ArrayRef<Type*> params) : ret(ret), params(params) { }
};

}

