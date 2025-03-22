#pragma once

#include "../pch.hpp"
#include <QtWidgets/qtoolbutton.h>

class Collapsible : public QWidget
{
    Q_OBJECT

public:
    Collapsible(const QString& title, const bool& _expanded = true, QWidget* parent = nullptr)
        : QWidget(parent)
    {
        QVBoxLayout* layout = new QVBoxLayout(this);
        layout->setSpacing(0);
        layout->setContentsMargins(0, 0, 0, 0);

        toggleButton = new QToolButton(this);
        toggleButton->setText(title);
        toggleButton->setCheckable(true);
        toggleButton->setChecked(true);
        toggleButton->setStyleSheet("QToolButton { border: none; font-weight: bold; background-color: #444; }");
        toggleButton->setArrowType(Qt::DownArrow);
        toggleButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        toggleButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        contentArea = new QFrame(this);
        contentArea->setFrameShape(QFrame::NoFrame);
        contentArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        contentArea->setStyleSheet("background-color: #555;");

        contentLayout = new QVBoxLayout(contentArea);
        contentLayout->setContentsMargins(10, 5, 10, 5);

        contentArea->setVisible(_expanded);
        toggleButton->setArrowType(_expanded ? Qt::DownArrow : Qt::RightArrow);

        layout->addWidget(toggleButton);
        layout->addWidget(contentArea);

        connect(toggleButton, &QToolButton::toggled, this, &Collapsible::ToggleContent);
    }

    void AddWidget(QWidget* widget)
    {
        contentLayout->addWidget(widget);
    }

	void AddSpacing(int spacing)
	{
		contentLayout->addSpacing(spacing);
	}

private slots:
    void ToggleContent(bool expanded)
    {
        contentArea->setVisible(expanded);
        toggleButton->setArrowType(expanded ? Qt::DownArrow : Qt::RightArrow);
    }

private:
    QToolButton* toggleButton;
    QFrame* contentArea;
    QVBoxLayout* contentLayout;
};
