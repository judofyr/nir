#include "IR.h"

using namespace nir;

void Block::addValue(P<Value> value) {
  value->addedTo(*this);
  values.push_back(std::move(value));
}

Type& Block::getType() {
  return values.back()->getType();
}

void Node::addedTo(Block& block) {
  assert(parent == nullptr);
  assert(block.owner);

  parent = &block;
  owner = block.owner;
}


