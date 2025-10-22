module;

#include <iostream>
#include "../macros.hpp"
#include "../ECSWrapper.hpp"
#include "QtWidgets/SceneHierarchy.hpp"

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
}