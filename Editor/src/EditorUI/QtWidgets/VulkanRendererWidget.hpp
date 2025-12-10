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
                // Creating groups

                std::vector<cp::InstanceGroup> groups;

                std::unordered_map<std::tuple<cp::Material*, cp::Mesh*, cp::MaterialInstance*>,
                    std::vector<cp::TransformData>,
                    Helper::Hash::TupleHash<cp::Material*, cp::Mesh*, cp::MaterialInstance*>> data;

				for (auto& entity : cp::CheckpointEditor::CurrentScene->entities)
                {
                    if (!entity->transform || !entity->meshRenderer) continue;

                    glm::mat4 modelMatrix = Transform::Helper::GetModelMatrix(*entity->transform);
                    glm::mat4 normalMatrix = glm::mat4(Transform::Helper::GetNormalMatrix(*entity->transform));

					if (entity->meshRenderer->materialInstance == nullptr || entity->meshRenderer->mesh == nullptr)
                    {
                        LOG_WARNING(MF("Entity '", entity->name, "' is missing a material instance or mesh, skipping rendering for this entity."));
                        continue;
                    }

                    data[std::make_tuple(entity->meshRenderer->materialInstance->GetMaterial().get(), entity->meshRenderer->mesh.get(), entity->meshRenderer->materialInstance.get())].push_back({modelMatrix, normalMatrix});
                }

                uint32_t instanceOffset = 0;

                for (auto& [tuple, tdata] : data)
                {
                    auto [material, mesh, materialInstance] = tuple;

                    groups.push_back({ material, materialInstance, mesh, tdata, instanceOffset });

                    instanceOffset += tdata.size();
                }

                std::sort(groups.begin(), groups.end(), [](const cp::InstanceGroup& a, const cp::InstanceGroup& b)
                    {
                        return a.material < b.material && a.mesh < b.mesh && a.materialInstance < b.materialInstance;
                    });

                editorCamera->UpdateUniformBuffer();
				renderer->UpdateCameraBuffer(editorCamera->GetUBOBuffer());

                if (renderer)
                {
                    renderer->Render(groups);
                }

                renderTimer.start();
            }
    };
}