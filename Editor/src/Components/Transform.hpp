#pragma once

#include "../pch.hpp"
#include "DirtyPattern.hpp"

struct Transform : public cp::IComponentBase, public DirtyPattern
{
	glm::vec3 position = glm::vec3(0.0f);
	glm::quat rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
	glm::vec3 scale = glm::vec3(1.0f);

	glm::mat4 matrix = glm::mat4(1.0f);
	glm::mat3 normalMatrix = glm::mat3(1.0f);

	static class Helper : public cp::ComponentBaseHelper<Transform>
	{
		void Translate(Transform& _component, const glm::vec3& _translation)
		{
			_component.position += _translation;
			_component.MarkDirty();
		}

		void SetPosition(Transform& _component, const glm::vec3& _position)
		{
			_component.position = _position;
			_component.MarkDirty();
		}

		void Rotate(Transform& _component, const glm::quat& _rotation)
		{
			_component.rotation = _rotation * _component.rotation;
			_component.MarkDirty();
		}

		void Rotate(Transform& _component, const glm::vec3& _rotation)
		{
			Rotate(_component, glm::quat(_rotation));
		}

		void SetRotation(Transform& _component, const glm::quat& _rotation)
		{
			_component.rotation = _rotation;
			_component.MarkDirty();
		}

		void SetRotation(Transform& _component, const glm::vec3& _rotation)
		{
			SetRotation(_component, glm::quat(_rotation));
		}

		void LookAt(Transform& _component, const glm::vec3& _target, const glm::vec3& _up = VEC3_UP)
		{
			glm::vec3 forward = glm::normalize(_target - _component.position);
			glm::vec3 fallback = _up;

			if (glm::abs(glm::dot(forward, _up)) > 0.9999f)
			{
				fallback = VEC3_RIGHT;

				if (glm::abs(forward.x) > 0.9999f)
				{
					fallback = VEC3_FORWARD;
				}
			}

			glm::vec3 right = glm::normalize(glm::cross(fallback, forward));
			glm::vec3 up = glm::cross(forward, right);

			_component.rotation = glm::quat_cast(glm::mat3(right, up, forward));
			_component.MarkDirty();
		}

		void SetScale(Transform& _component, const glm::vec3& _scale)
		{
			_component.scale = _scale;
			_component.MarkDirty();
		}

		const glm::vec3 GetPosition(const Transform& _component)
		{
			return _component.position;
		}

		const glm::quat GetRotation(const Transform& _component)
		{
			return _component.rotation;
		}

		const glm::vec3 GetScale(const Transform& _component)
		{
			return _component.scale;
		}

		const glm::vec3 GetForward(const Transform& _component)
		{
			return glm::normalize(_component.rotation * VEC3_FORWARD);
		}

		const glm::vec3 GetRight(const Transform& _component)
		{
			return glm::normalize(_component.rotation * VEC3_RIGHT);
		}

		const glm::vec3 GetUp(const Transform& _component)
		{
			return glm::normalize(_component.rotation * VEC3_UP);
		}

		void UpdateMatrix(Transform& _component)
		{
			if (!_component.dirty) return;

			_component.matrix = glm::translate(glm::mat4(1.0f), _component.position) * glm::mat4_cast(_component.rotation) * glm::scale(glm::mat4(1.0f), _component.scale);
			_component.normalMatrix = glm::mat3(glm::transpose(glm::inverse(_component.matrix)));

			_component.MarkClean();
		}

		const glm::mat4 GetModelMatrix(Transform& _component)
		{
			if (_component.dirty)
			{
				UpdateMatrix(_component);
			}

			return _component.matrix;
		}

		const glm::mat3 GetNormalMatrix(Transform& _component)
		{
			if (_component.dirty)
			{
				UpdateMatrix(_component);
			}

			return _component.normalMatrix;
		}
	};
};

class TransformSerializer : public cp::IComponentSerializer
{
public:
	TransformSerializer(cp::IComponentBase& _component) : IComponentSerializer(_component) {}

	void Serialize(cp::ISerializer& _serializer) const override
	{
		Transform& component = static_cast<Transform&>(this->component);
		_serializer.WriteVector3("position", component.position);
		_serializer.WriteQuaternion("rotation", component.rotation);
		_serializer.WriteVector3("scale", component.scale);
	}

	void Deserialize(cp::ISerializer& _serializer) override
	{
		Transform& component = static_cast<Transform&>(this->component);
		component.position = _serializer.ReadVector3("position", glm::vec3(0.0f));
		component.rotation = _serializer.ReadQuaternion("rotation", glm::quat(1.0f, 0.0f, 0.0f, 0.0f));
		component.scale = _serializer.ReadVector3("scale", glm::vec3(1.0f));
	}
};

class TransformWidget : public cp::ComponentWidget<Transform>
{
public:
	TransformWidget(Transform& _component, QWidget* _parent = nullptr) : ComponentWidget(_component, "Transform", _parent)
	{
		
	}

	void Initialize() override
	{
		layout->addSpacing(3);

		QLabel* positionLabel = new QLabel("Position", this);
		layout->addWidget(positionLabel);
		cp::Float3* positionField = new cp::Float3(&component.position, "Position", cp::LayoutDirection::Columns, this);
		layout->addWidget(positionField);

		QLabel* rotationLabel = new QLabel("Rotation", this);
		layout->addWidget(rotationLabel);
		cp::Quaternion* rotationField = new cp::Quaternion(&component.rotation, cp::LayoutDirection::Columns, this);
		layout->addWidget(rotationField);

		QLabel* scaleLabel = new QLabel("Scale", this);
		layout->addWidget(scaleLabel);
		cp::Float3* scaleField = new cp::Float3(&component.scale, "Scale", cp::LayoutDirection::Columns, this);
		layout->addWidget(scaleField);
	}
};