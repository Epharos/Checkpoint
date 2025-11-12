#pragma once

#include "../../pch.hpp"
#include "../../ECSWrapper.hpp"
#include "../../CheckpointEditor.hpp"

namespace cp {
    class VulkanRendererWidget : public QWindow {
#ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
#endif
        protected:
            cp::Renderer* renderer = nullptr;
			cp::SceneAsset* scene = nullptr;
			cp::PlatformQt platformQt;

			QVulkanInstance cpVulkanInstance;

            QTimer renderTimer;

			cp::Camera* editorCamera = nullptr;

			bool surfaceExposed = false;

        public:
            VulkanRendererWidget(cp::Renderer* _renderer, cp::SceneAsset* _scene = nullptr) : QWindow(), renderer(_renderer), scene(_scene)
            {
                setSurfaceType(QSurface::VulkanSurface);
				platformQt.Initialize(this);
				renderer->SetPlatform(&platformQt);

                cpVulkanInstance.setVkInstance(renderer->GetContext()->GetInstance());
				cpVulkanInstance.create();
				setVulkanInstance(&cpVulkanInstance);

				setMinimumSize(QSize(400, 400));

				editorCamera = new cp::Camera(renderer->GetContext(), renderer);
            }

            void Cleanup()
            {
                LOG_DEBUG("Cleaning up Vulkan Render Widget");
                renderTimer.stop();
                renderer->Cleanup();
				delete editorCamera;
                delete renderer;
                renderer = nullptr;
            }

            void SetScene(cp::SceneAsset* _scene)
            {
                scene = _scene;
			}

            void resizeEvent(QResizeEvent* _event) override
            {
                QWindow::resizeEvent(_event);
                if (renderer)
                {
                    renderer->TriggerSwapchainRecreation();
                }
            }

            void exposeEvent(QExposeEvent* _event) override
            {
				QWindow::exposeEvent(_event);

				if (!isExposed() || surfaceExposed)
                    return;

                if (!handle())
                {
					LOG_FATAL("Vulkan instance handle is null, cannot expose Vulkan surface");
					return;
                }

                if (!vulkanInstance())
                {
					LOG_FATAL("Vulkan instance is not set, cannot expose Vulkan surface");
					return;
                }

                renderer->SetSurface(QVulkanInstance::surfaceForWindow(this));
                renderer->Build();

                renderTimer.setInterval(16);
                connect(&renderTimer, &QTimer::timeout, this, &VulkanRendererWidget::UpdateRender);
                renderTimer.start();

				surfaceExposed = true;
            }

            void UpdateRender()
            {
                if (renderer)
                {
                    renderer->Render({});
                }

                renderTimer.start();
            }
    };
}