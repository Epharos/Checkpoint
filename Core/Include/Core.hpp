#pragma once

#include "../src/Context/VulkanContext.hpp"
#include "../src/Context/Platforms/PlatformGLFW.hpp"
#include "../src/Context/Platforms/PlatformQt.hpp"

#include "../src/Core/Scene.hpp"

#include "../src/Render/Renderer/Renderer.hpp"
#include "../src/Render/Setup/Frame.hpp"

#include "../src/Resources/Material.hpp"
#include "../src/Resources/Mesh.hpp"
#include "../src/Resources/Texture.hpp"
#include "../src/Resources/MaterialInstance.hpp"
#include "../src/Resources/ResourceManager.hpp"

#include "../src/Util/Clock.hpp"

#include "../src/Util/Serializers/JsonSerializer.hpp"
#include "../src/Util/ShaderCompiler/SlangCompiler.hpp"

#include "../src/ECS/EntityComponentSystem.hpp"

#include "../src/ECS/Component/ComponentBase.hpp"
#include "../src/ECS/Component/ComponentWidget.hpp"
#include "../src/ECS/Component/IComponentSerializer.hpp"
#include "../src/ECS/Component/ComponentRegistry.hpp"

#ifdef IN_EDITOR
#include "../src/Editor/ResourceDropLineEdit.hpp"

#include "../src/Editor/Project.hpp"
#endif