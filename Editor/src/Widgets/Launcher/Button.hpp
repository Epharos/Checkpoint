#pragma once

#include "../../pch.hpp"

enum ButtonImportance
{
	Primary,
	Secondary
};

class Button : public QPushButton
{
	Q_OBJECT

protected:
	ButtonImportance importance;

	std::string colorHex;

public:
	Button(const std::string& text, ButtonImportance importance, QWidget* parent = nullptr) : QPushButton(text.c_str(), parent), importance(importance)
	{
		if (importance == ButtonImportance::Primary)
			colorHex = "#007bff";
		else if (importance == ButtonImportance::Secondary)
			colorHex = "#6c757d";

		setStyleSheet("background-color:" + QString::fromStdString(colorHex) + "; border-color:#ddd; border-radius:5px; font-size:16px; color:#fff; font-weight:600;");
		setFixedSize(90, 30);
	}

	void SetCustomColor(const std::string& hex)
	{
		colorHex = hex;
		setStyleSheet("background-color:" + QString::fromStdString(colorHex) + "; border-color:#ddd; border-radius:5px; font-size:16px; color:#fff; font-weight:600;");
	}

	void SetCustomSize(int width, int height)
	{
		setFixedSize(width, height);
	}

	ButtonImportance GetImportance() const { return importance; }
	std::string GetColorHex() const { return colorHex; }
};