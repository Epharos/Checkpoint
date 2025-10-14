#pragma once

#if _DEBUG
#define USE_DEBUG_LAYER
#endif

#define IN_EDITOR

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
#include <typeindex>

#include <array>
#include <list>
#include <vector>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <optional>
#include <deque>

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

#ifdef IN_EDITOR
#include <QtWidgets/qwidget.h>
#include <QtWidgets/qboxlayout.h>
#include <QtWidgets/qlabel.h>
#include <QtWidgets/qgroupbox.h>

#include <Widgets.hpp>
#endif

#define VEC3_FORWARD glm::vec3(0.0f, 0.0f, 1.0f)
#define VEC3_UP glm::vec3(0.0f, 1.0f, 0.0f)
#define VEC3_RIGHT glm::vec3(1.0f, 0.0f, 0.0f)

#define NO_COPY(T) T(const T&) = delete; T& operator=(const T&) = delete; // This is a preprocessor definition that can be used to delete the copy constructor and the copy assignment operator of a class
#define NO_MOVE(T) T(T&&) noexcept = delete; T& operator=(T&&) noexcept = delete; // This is a preprocessor definition that can be used to delete the move constructor and the move assignment operator of a class
#define ALLOW_MOVE(T) T(T&&) noexcept = default; T& operator=(T&&) noexcept = default; // This is a preprocessor definition that can be used to allow the move constructor and the move assignment operator of a class