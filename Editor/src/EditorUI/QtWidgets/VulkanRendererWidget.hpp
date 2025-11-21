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
            cp::RendererInstance* renderer = nullptr;
			cp::SceneAsset* scene = nullptr;
			cp::PlatformQt platformQt;

			QVulkanInstance cpVulkanInstance;

            QTimer renderTimer;

			cp::Camera* editorCamera = nullptr;

			bool surfaceExposed = false;

        public:
            VulkanRendererWidget(cp::SceneAsset* _scene) : QWindow(), scene(_scene)
            {
                setSurfaceType(QSurface::VulkanSurface);
				platformQt.Initialize(this);
				renderer = new cp::RendererInstance(&CheckpointEditor::VulkanCtx, &platformQt, scene->renderer);

                cpVulkanInstance.setVkInstance(CheckpointEditor::VulkanCtx.GetInstance());
				cpVulkanInstance.create();
				setVulkanInstance(&cpVulkanInstance);

				setMinimumSize(QSize(400, 400));

				editorCamera = new cp::Camera(&CheckpointEditor::VulkanCtx, renderer);
            }

            void Cleanup()
            {
                LOG_DEBUG("Cleaning up Vulkan Render Widget");
                renderTimer.stop();
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
				renderer->ResetSwapchain();

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