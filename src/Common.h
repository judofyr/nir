#pragma once

#include <unordered_map>
#include <memory>
#include <string>
#include <cassert>

#include "ArrayRef.h"

namespace nir {

template<class T>
using P = std::unique_ptr<T>;

template<class K, class V>
using Dict = std::unordered_map<K, V>;

}

