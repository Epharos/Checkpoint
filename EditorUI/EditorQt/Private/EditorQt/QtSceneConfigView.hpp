#pragma once

#include "EditorQtCommon.hpp"

#include <QGroupBox>
#include <QListWidget>
#include <QPushButton>

namespace cp::editorqt
{
	class QtSceneConfigView final : public cp::editorui::ISceneConfigView, public IQtWidgetAccess, private QtWidgetBase
	{
	public:
		explicit QtSceneConfigView(std::string _id)
			: QtWidgetBase(std::move(_id))
			, root(new QWidget())
		{
			auto* layout = new QVBoxLayout(root.get());
			layout->setContentsMargins(4, 4, 4, 4);
			layout->setSpacing(6);

			auto* passGroup  = new QGroupBox("Render Passes");
			auto* passLayout = new QVBoxLayout(passGroup);
			passLayout->setContentsMargins(4, 4, 4, 4);
			passList = new QListWidget();
			passList->setObjectName("sceneConfig.passList");
			passLayout->addWidget(passList);
			layout->addWidget(passGroup);

			auto* systemGroup  = new QGroupBox("ECS Systems");
			auto* systemLayout = new QVBoxLayout(systemGroup);
			systemLayout->setContentsMargins(4, 4, 4, 4);
			systemList = new QListWidget();
			systemList->setObjectName("sceneConfig.systemList");
			systemLayout->addWidget(systemList);
			layout->addWidget(systemGroup);

			applyButton = new QPushButton("Apply");
			applyButton->setObjectName("sceneConfig.applyButton");
			layout->addWidget(applyButton);
			layout->addStretch();

			QObject::connect(applyButton, &QPushButton::clicked, [this]()
			{
				if (!applyHandler)
					return;

				std::vector<std::string> enabledPasses;
				for (int i = 0; i < passList->count(); ++i)
				{
					QListWidgetItem* item = passList->item(i);
					if (item->checkState() == Qt::Checked)
						enabledPasses.push_back(ToStdString(item->text()));
				}

				std::vector<std::string> enabledSystems;
				for (int i = 0; i < systemList->count(); ++i)
				{
					QListWidgetItem* item = systemList->item(i);
					if (item->checkState() == Qt::Checked)
						enabledSystems.push_back(ToStdString(item->text()));
				}

				applyHandler(std::move(enabledPasses), std::move(enabledSystems));
			});
		}

		[[nodiscard]] std::string_view GetId() const override { return QtWidgetBase::GetId(); }
		void SetVisible(bool _visible) override { QtWidgetBase::SetVisible(_visible); }
		[[nodiscard]] bool IsVisible() const override { return QtWidgetBase::IsVisible(); }
		void SetEnabled(bool _enabled) override { QtWidgetBase::SetEnabled(_enabled); }
		[[nodiscard]] bool IsEnabled() const override { return QtWidgetBase::IsEnabled(); }

		void SetRenderPassEntries(std::vector<cp::editorui::ISceneConfigView::RegistryEntry> _entries) override
		{
			passList->clear();
			for (const auto& entry : _entries)
			{
				auto* item = new QListWidgetItem(ToQString(entry.name), passList);
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
				item->setCheckState(entry.enabled ? Qt::Checked : Qt::Unchecked);
			}
		}

		void SetSystemEntries(std::vector<cp::editorui::ISceneConfigView::RegistryEntry> _entries) override
		{
			systemList->clear();
			for (const auto& entry : _entries)
			{
				auto* item = new QListWidgetItem(ToQString(entry.name), systemList);
				item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
				item->setCheckState(entry.enabled ? Qt::Checked : Qt::Unchecked);
			}
		}

		void SetApplyHandler(ApplyHandler _handler) override
		{
			applyHandler = std::move(_handler);
		}

		[[nodiscard]] QWidget* GetQWidget() const override { return root.get(); }

	private:
		std::unique_ptr<QWidget> root;
		QListWidget* passList = nullptr;
		QListWidget* systemList = nullptr;
		QPushButton* applyButton = nullptr;
		ApplyHandler applyHandler;
	};
}
