#include "RigidBody.hpp"

namespace cp
{
	std::string_view RigidBodyRegistrar::Name() const
	{
		return "RigidBody";
	}

	cp::ecs::TypeGuid RigidBodyRegistrar::ComponentGuid() const
	{
		return RigidBodyGuid;
	}

	void RigidBodyRegistrar::Register(cp::ecs::World& _world) const
	{
		_world.RegisterComponentSerialization<RigidBody>(
			RigidBodyGuid,
			1,
			std::string(Name()),
			[](const RigidBody& _value, cp::ISerializer& _serializer)
			{
				_serializer.WritePod(_value.speedX);
				_serializer.WritePod(_value.speedY);
				_serializer.WritePod(_value.speedZ);
			},
			[](RigidBody& _value, cp::IDeserializer& _deserializer)
			{
				return _deserializer.ReadPod(_value.speedX)
					&& _deserializer.ReadPod(_value.speedY)
					&& _deserializer.ReadPod(_value.speedZ);
			}
		);
	}

	std::string_view RigidBodyAuthoring::Name() const
	{
		return "RigidBody";
	}

	uint64_t RigidBodyAuthoring::ComponentGuid() const
	{
		return RigidBodyGuid;
	}

	bool RigidBodyAuthoring::HasComponent(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		return _world.HasComponent<RigidBody>(_entity);
	}

	bool RigidBodyAuthoring::AddComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (HasComponent(_world, _entity))
		{
			return false;
		}

		_world.AddComponent<RigidBody>(_entity, RigidBody{});
		return true;
	}

	bool RigidBodyAuthoring::RemoveComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return false;
		}

		return _world.RemoveComponent<RigidBody>(_entity);
	}

	std::vector<cp::AuthoringSectionDescriptor> RigidBodyAuthoring::BuildSections(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return {};
		}

		const RigidBody& rigidbody = _world.GetComponent<RigidBody>(_entity);
		return {
			cp::AuthoringSectionDescriptor{
				.id = "rigidbody",
				.title = "RigidBody",
				.fields = {
					cp::AuthoringFieldDescriptor{
						.id = "speed",
						.label = "Speed",
						.valueType = cp::AuthoringValueType::Vec3,
						.value = cp::AuthoringVec3{
							.x = rigidbody.speedX,
							.y = rigidbody.speedY,
							.z = rigidbody.speedZ
						}
					},
				}
			}
		};
	}

	bool RigidBodyAuthoring::ApplyValue(
		cp::ecs::World& _world,
		const cp::ecs::Entity& _entity,
		const std::string_view _fieldId,
		const cp::AuthoringValue& _value,
		std::string& _outError) const
	{
		if (!HasComponent(_world, _entity))
		{
			_outError = "RigidBody component is missing on selected entity.";
			return false;
		}

		RigidBody& rigidbody = _world.GetComponent<RigidBody>(_entity);
		const auto* vec3 = std::get_if<cp::AuthoringVec3>(&_value);

		if (vec3 == nullptr)
		{
			_outError = "RigidBody expects a Vec3 value.";
			return false;
		}

		rigidbody.speedX = vec3->x;
		rigidbody.speedY = vec3->y;
		rigidbody.speedZ = vec3->z;

		return true;
	}
}