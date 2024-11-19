#pragma once

#if _DEBUG
#define USE_DEBUG_LAYER
#endif

#include <iostream>
#include <string>
#include <memory>
#include <functional>
#include <algorithm>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <chrono>

#include <array>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <optional>

#include <thread>
#include <mutex>

#pragma warning(push)
#pragma warning(disable : 4996)
#include <vulkan/vulkan.hpp>
#pragma warning(pop)

#include "Util/Logger.hpp"

#include "GLFW/glfw3.h"