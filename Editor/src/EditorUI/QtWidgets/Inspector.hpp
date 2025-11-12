#pragma once

#include <QtWidgets/qpushbutton.h>
#include <QtCore/qstring.h>
#include <QtGui/qevent.h>
#include "../../ECSWrapper.hpp"
#include "Helper.hpp"
#include "ColoredIconButton.hpp"
#include <format>

namespace cp {
	class Inspector : public QWidget
	{
#ifndef BUILDING_PLUGIN_LOADER
		Q_OBJECT
#endif
	protected:
		QVBoxLayout* layout;
		QLabel* titleLabel;

		std::unordered_map<std::string, std::function<void(const std::string&)>> fileInspector;
		void* readFile = nullptr;

		void ShowMaterial(const std::string& _path);

	public:
		explicit Inspector(QWidget* _parent = nullptr);
		void Clear();
		void ShowEntity(cp::EntityAsset* _entity);
		void ShowFile(const std::string& _path);
	};
}