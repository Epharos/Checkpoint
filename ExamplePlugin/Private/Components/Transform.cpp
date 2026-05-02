#include "Transform.hpp"

namespace cp
{
	std::string_view TransformRegistrar::Name() const
	{
		return "Transform";
	}

	cp::ecs::TypeGuid TransformRegistrar::ComponentGuid() const
	{
		return TransformGuid;
	}

	void TransformRegistrar::Register(cp::ecs::World& _world) const
	{
		_world.RegisterComponentSerialization<Transform>(
			TransformGuid,
			1,
			std::string(Name()),
			[](const Transform& _value, cp::ISerializer& _serializer)
			{
				_serializer.WritePod(_value.x);
				_serializer.WritePod(_value.y);
				_serializer.WritePod(_value.z);
				_serializer.WritePod(_value.pitch);
				_serializer.WritePod(_value.yaw);
				_serializer.WritePod(_value.roll);
				_serializer.WritePod(_value.scaleX);
				_serializer.WritePod(_value.scaleY);
				_serializer.WritePod(_value.scaleZ);
			},
			[](Transform& _value, cp::IDeserializer& _deserializer)
			{
				return _deserializer.ReadPod(_value.x)
					&& _deserializer.ReadPod(_value.y)
					&& _deserializer.ReadPod(_value.z)
					&& _deserializer.ReadPod(_value.pitch)
					&& _deserializer.ReadPod(_value.yaw)
					&& _deserializer.ReadPod(_value.roll)
					&& _deserializer.ReadPod(_value.scaleX)
					&& _deserializer.ReadPod(_value.scaleY)
					&& _deserializer.ReadPod(_value.scaleZ);
			}
		);
	}

	std::string_view TransformAuthoring::Name() const
	{
		return "Transform";
	}

	uint64_t TransformAuthoring::ComponentGuid() const
	{
		return TransformGuid;
	}

	bool TransformAuthoring::HasComponent(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		return _world.HasComponent<Transform>(_entity);
	}

	bool TransformAuthoring::AddComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (HasComponent(_world, _entity))
		{
			return false;
		}

		_world.AddComponent<Transform>(_entity, Transform{});
		return true;
	}

	bool TransformAuthoring::RemoveComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return false;
		}

		return _world.RemoveComponent<Transform>(_entity);
	}

	std::vector<cp::AuthoringSectionDescriptor> TransformAuthoring::BuildSections(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return {};
		}

		const Transform& transform = _world.GetComponent<Transform>(_entity);
		return {
			cp::AuthoringSectionDescriptor{
				.id = "transform",
				.title = "Transform",
				.fields = {
					cp::AuthoringFieldDescriptor{
						.id = "position",
						.label = "Position",
						.valueType = cp::AuthoringValueType::Vec3,
						.value = cp::AuthoringVec3{
							.x = transform.x,
							.y = transform.y,
							.z = transform.z
						}
					},
					cp::AuthoringFieldDescriptor{
						.id = "rotation",
						.label = "Rotation (Pitch/Yaw/Roll)",
						.valueType = cp::AuthoringValueType::Vec3,
						.value = cp::AuthoringVec3{
							.x = transform.pitch,
							.y = transform.yaw,
							.z = transform.roll
						}
					},
					cp::AuthoringFieldDescriptor{
						.id = "scale",
						.label = "Scale",
						.valueType = cp::AuthoringValueType::Vec3,
						.value = cp::AuthoringVec3{
							.x = transform.scaleX,
							.y = transform.scaleY,
							.z = transform.scaleZ
						}
					}
				}
			}
		};
	}

	bool TransformAuthoring::ApplyValue(
		cp::ecs::World& _world,
		const cp::ecs::Entity& _entity,
		const std::string_view _fieldId,
		const cp::AuthoringValue& _value,
		std::string& _outError) const
	{
		if (!HasComponent(_world, _entity))
		{
			_outError = "Transform component is missing on selected entity.";
			return false;
		}

		Transform& transform = _world.GetComponent<Transform>(_entity);
		const auto* vec3 = std::get_if<cp::AuthoringVec3>(&_value);
		if (vec3 == nullptr)
		{
			_outError = "Transform expects a Vec3 value.";
			return false;
		}

		if (_fieldId == "position")
		{
			transform.x = static_cast<float>(vec3->x);
			transform.y = static_cast<float>(vec3->y);
			transform.z = static_cast<float>(vec3->z);
			return true;
		}
		if (_fieldId == "rotation")
		{
			transform.pitch = static_cast<float>(vec3->x);
			transform.yaw = static_cast<float>(vec3->y);
			transform.roll = static_cast<float>(vec3->z);
			return true;
		}
		if (_fieldId == "scale")
		{
			transform.scaleX = static_cast<float>(vec3->x);
			transform.scaleY = static_cast<float>(vec3->y);
			transform.scaleZ = static_cast<float>(vec3->z);
			return true;
		}

		_outError = "Unknown Transform field: " + std::string(_fieldId);
		return false;
	}
}
