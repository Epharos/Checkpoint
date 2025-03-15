#pragma once

#include "pch.hpp"

#include "Renderers/MinimalistRenderer.hpp"

class VulkanWindow : public QWindow
{
    Q_OBJECT
protected:
    cp::Scene* currentScene;

	QTimer renderTimer;

public:
    VulkanWindow(cp::Scene* _scene) : QWindow(), currentScene(_scene)
    {
		setSurfaceType(QSurface::VulkanSurface);

		renderTimer.setInterval(16);
		connect(&renderTimer, &QTimer::timeout, this, &VulkanWindow::UpdateRender);
		renderTimer.start();
    }

    void exposeEvent(QExposeEvent* _event) override
    {
        Q_UNUSED(_event);
    }

	void UpdateRender()
	{
        if (currentScene && currentScene->GetRenderer())
        {
            currentScene->GetRenderer()->Render({});
        }

		renderTimer.start();
	}

	void SetScene(cp::Scene* _scene) 
    {
        currentScene = _scene;
    }
};