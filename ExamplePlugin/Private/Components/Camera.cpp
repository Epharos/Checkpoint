#include "Camera.hpp"

namespace cp
{
	std::string_view CameraRegistrar::Name() const
	{
		return "Camera";
	}

	cp::ecs::TypeGuid CameraRegistrar::ComponentGuid() const
	{
		return CameraGuid;
	}

	void CameraRegistrar::Register(cp::ecs::World& _world) const
	{
		_world.RegisterComponentSerialization<Camera>(
			CameraGuid,
			1,
			std::string(Name()),
			[](const Camera& _value, cp::ISerializer& _serializer)
			{
				_serializer.WritePod(_value.fovYDegrees);
				_serializer.WritePod(_value.nearPlane);
				_serializer.WritePod(_value.farPlane);
				_serializer.WritePod(_value.isPrimary);
				_serializer.WritePod(_value.enabled);
			},
			[](Camera& _value, cp::IDeserializer& _deserializer)
			{
				return _deserializer.ReadPod(_value.fovYDegrees)
					&& _deserializer.ReadPod(_value.nearPlane)
					&& _deserializer.ReadPod(_value.farPlane)
					&& _deserializer.ReadPod(_value.isPrimary)
					&& _deserializer.ReadPod(_value.enabled);
			}
		);
	}

	std::string_view CameraAuthoring::Name() const
	{
		return "Camera";
	}

	uint64_t CameraAuthoring::ComponentGuid() const
	{
		return CameraGuid;
	}

	bool CameraAuthoring::HasComponent(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		return _world.HasComponent<Camera>(_entity);
	}

	bool CameraAuthoring::AddComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (HasComponent(_world, _entity))
		{
			return false;
		}

		_world.AddComponent<Camera>(_entity, Camera{});
		return true;
	}

	bool CameraAuthoring::RemoveComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return false;
		}

		return _world.RemoveComponent<Camera>(_entity);
	}

	std::vector<cp::AuthoringSectionDescriptor> CameraAuthoring::BuildSections(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return {};
		}

		const Camera& camera = _world.GetComponent<Camera>(_entity);
		return {
			cp::AuthoringSectionDescriptor{
				.id = "camera",
				.title = "Camera",
				.fields = {
					cp::AuthoringFieldDescriptor{
						.id = "fovYDegrees",
						.label = "FOV Y (degrees)",
						.valueType = cp::AuthoringValueType::Float,
						.value = static_cast<double>(camera.fovYDegrees)
					},
					cp::AuthoringFieldDescriptor{
						.id = "nearPlane",
						.label = "Near Plane",
						.valueType = cp::AuthoringValueType::Float,
						.value = static_cast<double>(camera.nearPlane)
					},
					cp::AuthoringFieldDescriptor{
						.id = "farPlane",
						.label = "Far Plane",
						.valueType = cp::AuthoringValueType::Float,
						.value = static_cast<double>(camera.farPlane)
					},
					cp::AuthoringFieldDescriptor{
						.id = "isPrimary",
						.label = "Primary Camera",
						.valueType = cp::AuthoringValueType::Bool,
						.value = camera.isPrimary
					},
					cp::AuthoringFieldDescriptor{
						.id = "enabled",
						.label = "Enabled",
						.valueType = cp::AuthoringValueType::Bool,
						.value = camera.enabled
					}
				}
			}
		};
	}

	bool CameraAuthoring::ApplyValue(
		cp::ecs::World& _world,
		const cp::ecs::Entity& _entity,
		const std::string_view _fieldId,
		const cp::AuthoringValue& _value,
		std::string& _outError) const
	{
		if (!HasComponent(_world, _entity))
		{
			_outError = "Camera component is missing on selected entity.";
			return false;
		}

		Camera& camera = _world.GetComponent<Camera>(_entity);
		if (_fieldId == "isPrimary" || _fieldId == "enabled")
		{
			const bool* boolValue = std::get_if<bool>(&_value);
			if (boolValue == nullptr)
			{
				_outError = "Camera boolean field expects a bool value.";
				return false;
			}

			if (_fieldId == "isPrimary")
			{
				camera.isPrimary = *boolValue;
			}
			else
			{
				camera.enabled = *boolValue;
			}
			return true;
		}

		const double* floatValue = std::get_if<double>(&_value);
		if (floatValue == nullptr)
		{
			_outError = "Camera numeric field expects a float value.";
			return false;
		}

		if (_fieldId == "fovYDegrees")
		{
			camera.fovYDegrees = static_cast<float>(*floatValue);
			return true;
		}
		if (_fieldId == "nearPlane")
		{
			camera.nearPlane = static_cast<float>(*floatValue);
			return true;
		}
		if (_fieldId == "farPlane")
		{
			camera.farPlane = static_cast<float>(*floatValue);
			return true;
		}

		_outError = "Unknown Camera field: " + std::string(_fieldId);
		return false;
	}
}
