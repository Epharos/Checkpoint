#pragma once

#if _DEBUG
#define USE_DEBUG_LAYER
#endif

#define IN_EDITOR // This is a preprocessor definition that can be used to check if the code is being compiled in the editor or in the game

#define GLM_FORCE_RADIANS

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

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Util/Logger.hpp"
#include "Helpers/Helpers.hpp"

#include "GLFW/glfw3.h"

#include "ECS.hpp"

#define VEC3_FORWARD glm::vec3(0.0f, 0.0f, 1.0f)
#define VEC3_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define VEC3_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)