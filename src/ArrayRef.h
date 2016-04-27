#pragma once

#include <vector>

namespace nir {

template<class T>
struct ArrayRef {
  T const* base;
  int size;

  void copy(std::vector<T>& vec) {
    for (int i = 0; i < size; i++) {
      vec.emplace_back(base[i]);
    }
  }

  operator std::vector<T>() const {
    return std::vector<T>(base, base+size);
  }

  template<size_t N>
  ArrayRef(const T(&arr)[N]) {
    base = &arr[0];
    size = N;
  }

  T const* begin() {
    return base;
  }

  T const* end() {
    return base + size;
  }
};

}
