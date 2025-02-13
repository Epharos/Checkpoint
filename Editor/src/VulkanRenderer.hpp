#pragma once

#include <QtGui/qvulkanwindow.h>

#include <Core.hpp>

#include "Renderers/MinimalistRenderer.hpp"

class VulkanRenderer : public QVulkanWindowRenderer 
{
public:
	VulkanRenderer(QVulkanWindow* _window, Context::VulkanContext* _context, Core::Scene* _scene) : vulkanWindow(_window), vulkanContext(_context), currentScene(_scene)
    {
        qDebug() << "VulkanRenderer initialized.";
    }

    void initResources() override 
    {
        qDebug() << "Initializing Vulkan resources...";
		MinimalistRenderer* renderer = new MinimalistRenderer(vulkanContext);
		renderer->Build();

        // TODO: NEED TO INITIALIZE THE SCENE
    }

    void releaseResources() override 
    {
        qDebug() << "Releasing Vulkan resources...";
		currentScene->Cleanup();
    }

    void startNextFrame() override 
    {
        qDebug() << "Rendering frame...";
        vulkanWindow->requestUpdate();
    }

private:
	Core::Scene* currentScene;
    QVulkanWindow* vulkanWindow;
	Context::VulkanContext* vulkanContext;
};