#pragma once

#include "pch.hpp"

namespace cp {
    class VulkanRendererWidget : public QWindow {
#ifndef BUILDING_PLUGIN_LOADER
        Q_OBJECT
#endif
        protected:
            cp::Renderer* renderer = nullptr;
			cp::SceneAsset* scene = nullptr;

            QTimer renderTimer;

        public:
            VulkanRendererWidget(cp::Renderer* _renderer, cp::SceneAsset* _scene = nullptr) : QWindow(), renderer(_renderer), scene(_scene)
            {
                setSurfaceType(QSurface::VulkanSurface);

                renderTimer.setInterval(16);
                connect(&renderTimer, &QTimer::timeout, this, &VulkanRendererWidget::UpdateRender);
                renderTimer.start();
            }

            void exposeEvent(QExposeEvent* _event) override
            {
                Q_UNUSED(_event);
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