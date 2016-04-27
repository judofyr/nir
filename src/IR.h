#pragma once

#include "Common.h"
#include "Type.h"

#include <unordered_set>
namespace nir {

struct Value;
struct Block;
struct FnValue;

struct Node {
  int passLevel = 0;
  Block* parent = nullptr;
  FnValue* owner = nullptr;
  virtual void addedTo(Block& block);
  void *extra = nullptr;
};

struct Block : Node {
  std::vector<P<Value>> values;

  Block() { }

  Type& getType();

  void addValue(P<Value>);

  template<class T, class... U>
  T& insert(U&&... u) {
    auto val = std::make_unique<T>(std::forward<U>(u)...);
    auto ptr = val.get();
    addValue(std::move(val));
    return *ptr;
  }

  bool isEmpty() {
    return values.size() == 0;
  }
};

struct Value : Node {
  std::string name;
  std::unordered_set<Value*> users;
  std::vector<Value*> uses;

  Value() { }
  Value(const Value&) = delete;
  Value(Value&&) = default;

  void pushUse(Value& other) {
    other.users.insert(this);
    uses.push_back(&other);
  }

  virtual ~Value() {
    for (auto other : uses) {
      other->users.erase(this);
    }
  }

  virtual Type& getType() = 0;
  virtual bool eq(Value& other) {
    return this == &other;
  }
  virtual bool mustPreserve() {
    return false;
  }
};

struct Constant : Value {
  virtual P<Constant> copy() = 0;
};

struct IntValue : Constant {
  IntType& type;
  uint64_t value;

  IntValue(IntType& type, uint64_t value) : type(type), value(value) { }

  P<Constant> copy() override {
    return std::make_unique<IntValue>(type, value);
  }

  IntType& getType() override {
    return type;
  }

  bool eq(Value& other) override {
    auto &oint = dynamic_cast<IntValue&>(other);
    return value == oint.value;
  }
};

struct FloatValue : Constant {
  FloatType& type;

  uint8_t sign : 1;
  uint16_t exp : 11;
  uint64_t frac : 52;

  FloatValue(FloatType& type) : type(type) { }

  FloatType& getType() override {
    return type;
  }

  P<Constant> copy() override {
    auto res = std::make_unique<FloatValue>(type);
    res->sign = sign;
    res->exp = exp;
    res->frac = frac;
    return std::move(res);
  }
};

struct CopyValue : Value {
  CopyValue(Value& original) {
    pushUse(original);
  }

  Value& getOriginal() {
    return *uses[0];
  }

  Type& getType() override {
    return getOriginal().getType();
  }
};

struct BranchValue : Value {
  Block tbranch;
  Block fbranch;

  BranchValue(Value& cond) {
    pushUse(cond);
  }

  Value& getCond() {
    return *uses[0];
  }

  Type& getType() override {
    return tbranch.getType();
  }

  void addedTo(Block& block) override {
    Node::addedTo(block);
    tbranch.addedTo(block);
    fbranch.addedTo(block);
  }
};

struct FnParam : Value {
  Type& type;

  FnParam(Type& type) : type(type) { }

  Type& getType() override {
    return type;
  }
};

struct FnCallValue;

struct FnValue : Value {
  FnType type;
  std::vector<FnParam> params;
  Block body;

  FnValue(FnType& type) : type(type) {
    for (auto &paramtype : type.params) {
      params.emplace_back(*paramtype);
    }
  }

  std::vector<FnCallValue*> calls;

  Type& getType() override {
    return type;
  }

  void addedTo(Block& block) override {
    Node::addedTo(block);
    body.parent = &body;
  }

  virtual bool isComplete() {
    return true;
  }
};

struct TopFnValue : FnValue {
  bool isImported = false;
  bool isExported = false;

  TopFnValue(FnType& type) : FnValue(type) { }

  bool mustPreserve() override {
    return isExported;
  }

  bool isComplete() override {
    return !isImported;
  }
};

struct FnCallValue : Value {
  FnType* fntype;

  FnCallValue(Value& fn, ArrayRef<Value*> argsref) {
    fntype = dynamic_cast<FnType*>(&fn.getType());
    assert(fntype);

    pushUse(fn);
    for (auto arg : argsref) {
      pushUse(*arg);
    }
  }

  Value& getFn() { return *uses[0]; }
  std::vector<Value*>::iterator begin_args() { return ++uses.begin(); }
  std::vector<Value*>::iterator end_args() { return uses.end(); }

  Type& getType() override {
    return fntype->ret;
  }
};



}

