#pragma once

#include <QtGui/qvulkanwindow.h>

#include <Core.hpp>

#include "Renderers/MinimalistRenderer.hpp"

class VulkanRenderer : public QVulkanWindowRenderer 
{
public:
	VulkanRenderer(QVulkanWindow* _window, Core::Scene* _scene) : vulkanWindow(_window), currentScene(_scene)
    {
        //qDebug() << "VulkanRenderer initialized.";
    }

    void initResources() override 
    {
        //qDebug() << "Initializing Vulkan resources...";
    }

    void releaseResources() override 
    {
        //qDebug() << "Releasing Vulkan resources...";
        if(currentScene) currentScene->Cleanup();
    }

    void startNextFrame() override 
    {
        //qDebug() << "Rendering frame...";
        if (currentScene) currentScene->GetRenderer()->Render({});
        vulkanWindow->frameReady();
        vulkanWindow->requestUpdate();
    }

	void SetScene(Core::Scene* _scene) { currentScene = _scene; }

private:
	Core::Scene* currentScene;
    QVulkanWindow* vulkanWindow;
};

class VulkanWindow : public QVulkanWindow
{
    Q_OBJECT
protected:
    Core::Scene* currentScene;
    VulkanRenderer* windowRenderer;

public:
    VulkanWindow(Core::Scene* _scene) : QVulkanWindow(), currentScene(_scene)
    {

    }

    QVulkanWindowRenderer* createRenderer() override
    {
        return (windowRenderer = new VulkanRenderer(this, currentScene));
    }

	void SetScene(Core::Scene* _scene) 
    {
        currentScene = _scene;
		windowRenderer->SetScene(currentScene);
    }
};