#include "DeadCodeElim.h"

using namespace nir;

template<class T>
static void process(std::vector<T>& vals, bool preserve_tail) {
  auto fst = vals.begin();
  auto lst = vals.end();
  bool is_tail = preserve_tail;

  while (fst != lst) {
    --lst;
    auto &value = *lst;
    if (!is_tail && value->users.size() == 0 && !value->mustPreserve()) {
      vals.erase(lst);
      continue;
    }

    if (auto vfn = dynamic_cast<FnValue*>(&*value)) {
      process(vfn->body.values, true);
    }

    if (auto vbr = dynamic_cast<BranchValue*>(&*value)) {
      process(vbr->tbranch.values, true);
      process(vbr->fbranch.values, true);
    }

    is_tail = false;
  }
}

void DeadCodeElim::run() {
  process(module.fns, false);
}

