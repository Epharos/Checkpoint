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
                editorCamera->SetPerspective(60.f, renderer->GetPlatform()->GetAspectRatio(), 0.01f, 300.f);
				//editorCamera->Translate(glm::vec3(0.0f, 0.0f, 5.0f));
				//editorCamera->LookAt(glm::vec3(0.0f));
                renderer->UpdateCameraBuffer(editorCamera->GetUBOBuffer());
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

                    if (editorCamera)
                    {
                        editorCamera->SetPerspective(60.f, renderer->GetPlatform()->GetAspectRatio(), 0.01f, 300.f);
                    }
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
				LOG_DEBUG("--- Vulkan Renderer Widget Update Render ---");
                LOG_DEBUG("");

                auto Mat4ToString = [](const glm::mat4& mat)
                    {
                        std::string result;
                        for (int i = 0; i < 4; ++i)
                        {
                            result += "| ";
                            for (int j = 0; j < 4; ++j)
                            {
                                result += std::to_string(mat[i][j]) + " ";
                            }
                            result += "|\n";
                        }
                        return result;
                    };

                auto Vec3ToString = [](const glm::vec3& vec)
                    {
                        return "(" + std::to_string(vec.x) + ", " + std::to_string(vec.y) + ", " + std::to_string(vec.z) + ")";
                    };

                if(editorCamera)
                {
					LOG_DEBUG("Editor Camera Position: " + Vec3ToString(editorCamera->GetPosition()));
					LOG_DEBUG("Editor Camera View Matrix:\n" + Mat4ToString(editorCamera->GetViewMatrix()));
					LOG_DEBUG("Editor Camera Projection Matrix:\n" + Mat4ToString(editorCamera->GetProjectionMatrix()));
					LOG_DEBUG("Editor Camera View-Projection Matrix:\n" + Mat4ToString(editorCamera->GetProjectionMatrix() * editorCamera->GetViewMatrix()));
                }

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

					LOG_DEBUG(MF("Entity '", entity->name, "' Model Matrix:\n", Mat4ToString(modelMatrix)));
                    LOG_DEBUG(MF("Entity '", entity->name, "' Position:\n", Vec3ToString(entity->transform->position)));

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

                if (renderer)
                {
                    renderer->Render(groups);
                }

                renderTimer.start();

                LOG_DEBUG("");
                LOG_DEBUG("");
                LOG_DEBUG("");
            }
    };
}