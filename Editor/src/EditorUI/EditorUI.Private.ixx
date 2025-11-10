module;

#include <iostream>
#include "../macros.hpp"
#include "../ECSWrapper.hpp"
#include "QtWidgets/SceneHierarchy.hpp"
#include "QtWidgets/Inspector.hpp"
#include "QtWidgets/VulkanRendererWidget.hpp"

export module EditorUI:Private;

import :Core;

export namespace cp {
	class ISceneHierarchy : public IWidget {
	public:
		virtual void UpdateScene(SceneAsset* _scene) const = 0;
	};

	class QtSceneHierarchy : public ISceneHierarchy {
	public:
		QtSceneHierarchy(cp::SceneAsset* _scene = nullptr) {
			sceneHierarchy = new cp::SceneHierarchy(_scene);
		}

		virtual ~QtSceneHierarchy() = default;

		virtual void  SetVisible(bool visible) noexcept {
			sceneHierarchy->setVisible(visible);
		}
		virtual bool IsVisible() const noexcept {
			return sceneHierarchy->isVisible();
		}

		virtual void SetEnabled(bool enabled) noexcept {
			sceneHierarchy->setEnabled(enabled);
		}

		virtual bool IsEnabled() const noexcept {
			return sceneHierarchy->isEnabled();
		}

		virtual void* NativeHandle() const noexcept {
			return static_cast<void*>(sceneHierarchy);
		}

		virtual void UpdateScene(SceneAsset* _scene) const {
			sceneHierarchy->InitTree(_scene);
		}
	protected:
		SceneHierarchy* sceneHierarchy;
	};

	class IInspector : public IWidget {
		public:
		virtual void ShowEntity(EntityAsset* _entity) = 0;
	};

	class QtInspector : public IInspector {
	public:
		QtInspector() {
			inspector = new cp::Inspector();
		}
		virtual ~QtInspector() = default;
		virtual void  SetVisible(bool visible) noexcept {
			inspector->setVisible(visible);
		}
		virtual bool IsVisible() const noexcept {
			return inspector->isVisible();
		}
		virtual void SetEnabled(bool enabled) noexcept {
			inspector->setEnabled(enabled);
		}
		virtual bool IsEnabled() const noexcept {
			return inspector->isEnabled();
		}
		virtual void* NativeHandle() const noexcept {
			return static_cast<void*>(inspector);
		}
		virtual void ShowEntity(EntityAsset* _entity) {
			inspector->ShowEntity(_entity);
		}
	protected:
		Inspector* inspector;
	};

	class IViewport : public IWidget {
	public:
		virtual void SetScene(SceneAsset* _scene) = 0;
	};

	class QtViewport : public IViewport {
		public:
		QtViewport(cp::Renderer* _renderer, cp::SceneAsset* _scene = nullptr) {
			viewport = new cp::VulkanRendererWidget(_renderer, _scene);
			viewportContainer = QWidget::createWindowContainer(viewport);
			viewportContainer->setProperty("viewportHandle", QVariant::fromValue<void*>(static_cast<void*>(viewport)));
		}

		virtual ~QtViewport() = default;

		virtual void  SetVisible(bool visible) noexcept {
			viewport->setVisible(visible);
		}

		virtual bool IsVisible() const noexcept {
			return viewport->isVisible();
		}

		virtual void SetEnabled(bool enabled) noexcept {
			LOG_WARNING("Viewport enable/disable not implemented");
		}

		virtual bool IsEnabled() const noexcept {
			return true;
		}

		virtual void* NativeHandle() const noexcept {
			return static_cast<void*>(viewportContainer);
		}

		virtual void SetScene(SceneAsset* _scene) {
			viewport->SetScene(_scene);
		}
	protected:
		VulkanRendererWidget* viewport;
		QWidget* viewportContainer;
	};
}