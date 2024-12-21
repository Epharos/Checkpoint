#pragma once

#include <iostream>

#include <bitset>
#include <functional>

#include <vector>
#include <map>
#include <set>
#include <queue>
#include <deque>
#include <array>
#include <unordered_map>
#include <typeindex>

#include "Entity.hpp"

const uint32_t MAX_COMPONENTS = 32;
const uint32_t MAX_ENTITIES = 10000;

template<uint32_t N>
using Signature = std::bitset<N>;