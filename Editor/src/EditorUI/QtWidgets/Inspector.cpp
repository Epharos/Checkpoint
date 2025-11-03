#include "Inspector.hpp"
#include "../../Components/ComponentView.hpp"
#include "../../Components/Transform.hpp"

import EditorUI;

cp::Inspector::Inspector(QWidget* _parent)
{
	QWidget::QWidget(_parent);
	layout = new QVBoxLayout(this);
	this->setLayout(layout);
	titleLabel = new QLabel("Select an entity", this);
	layout->addWidget(titleLabel);
	layout->setAlignment(Qt::AlignTop);
}

void cp::Inspector::Clear()
{
	QLayoutItem* item;

	while ((item = layout->takeAt(1)) != nullptr) {
		if (item->widget()) {
			delete item->widget();
		}

		delete item;
	}

	titleLabel->setText("Select an entity");
}

void cp::Inspector::ShowEntity(cp::EntityAsset* _entity)
{
	Clear();

	if (!_entity) {
		return;
	}

	titleLabel->setText(QString::fromStdString(_entity->name.empty() ? "Entity" : _entity->name));

	cp::QtEditorUIFactory factory;

	for (size_t i = 0; i < _entity->components.size(); ++i) {
		auto view = cp::ComponentViewRegistry::GetInstance().CreateView(_entity->components[i]);
		auto collapsible = factory.CreateCollapsible().release();
		collapsible->SetTitle(view->GetName());
		collapsible->SetContent(view->Render(&factory));
		layout->addWidget(static_cast<QWidget*>(collapsible->NativeHandle()));

		if (i != _entity->components.size() - 1) {
			QFrame* line = new QFrame(this);
			line->setFrameShape(QFrame::HLine);
			line->setFrameShadow(QFrame::Sunken);
			line->setStyleSheet("background-color: #3E465A; margin-top: 4px; margin-bottom: 4px; min-height: 1px; max-height: 1px;");
			layout->addWidget(line);
		}
	}
}
