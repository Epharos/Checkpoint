#pragma once

#include "pch.hpp"
#include "Vector.hpp"

namespace cp
{
	class Float2 : public cp::VectorWidget<glm::vec<2, glm::f32, glm::defaultp>, 2>
	{
		Q_OBJECT

	public:
		Float2(glm::vec<2, glm::f32, glm::defaultp>* _vec2, const std::string& _fieldName, const cp::LayoutDirection& _layoutDir = cp::LayoutDirection::Columns, QWidget* parent = nullptr)
			: VectorWidget(_vec2, _fieldName, _layoutDir, parent)
		{
		}
	};

	class Float3 : public cp::VectorWidget<glm::vec<3, glm::f32, glm::defaultp>, 3>
	{
		Q_OBJECT

	public:
		Float3(glm::vec<3, glm::f32, glm::defaultp>* _vec3, const std::string& _fieldName, const cp::LayoutDirection& _layoutDir = cp::LayoutDirection::Columns, QWidget* parent = nullptr)
			: VectorWidget(_vec3, _fieldName, _layoutDir, parent)
		{
		}
	};

	class Float4 : public cp::VectorWidget<glm::vec<4, glm::f32, glm::defaultp>, 4>
	{
		Q_OBJECT

	public:
		Float4(glm::vec<4, glm::f32, glm::defaultp>* _vec4, const std::string& _fieldName, const cp::LayoutDirection& _layoutDir = cp::LayoutDirection::Columns, QWidget* parent = nullptr)
			: VectorWidget(_vec4, _fieldName, _layoutDir, parent)
		{
		}
	};
}

//TODO : Add more vector type (int, uint, double, half, ...)