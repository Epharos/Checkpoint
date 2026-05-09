#include "MeshRenderer.hpp"

#include <Common/IO/FileHelper.hpp>

#include "ComponentRegistrationContext.hpp"

namespace cp
{
	std::string_view MeshRendererRegistrar::Name() const
	{
		return "MeshRenderer";
	}

	cp::ecs::TypeGuid MeshRendererRegistrar::ComponentGuid() const
	{
		return MeshRendererGuid;
	}

	void MeshRendererRegistrar::Register(cp::ecs::World& _world) const
	{
		_world.RegisterComponentSerialization<MeshRenderer>(
			MeshRendererGuid,
			1,
			std::string(Name()),
			[](const MeshRenderer& _value, cp::ISerializer& _serializer)
			{
				_serializer.WriteString(GetRelativePath(_value.meshId.GetAssetID()).string());
				_serializer.WritePod(_value.visible);
			},
			[](MeshRenderer& _value, cp::IDeserializer& _deserializer)
			{
				std::string meshAssetPath;
				if (!_deserializer.ReadString(meshAssetPath)
					|| !_deserializer.ReadPod(_value.visible))
				{
					return false;
				}

				if (meshAssetPath.empty())
				{
					_value.meshId = AssetHandle<Mesh>{};
					return true;
				}

				auto& meshManager = cp::GetComponentRegistrationContext().GetAssetRegistry().Get<Mesh>();
				_value.meshId = meshManager.Load(FindFileInParentTree(meshAssetPath));
				return _value.meshId.IsValid();
			}
		);
	}

	std::string_view MeshRendererAuthoring::Name() const
	{
		return "MeshRenderer";
	}

	uint64_t MeshRendererAuthoring::ComponentGuid() const
	{
		return MeshRendererGuid;
	}

	bool MeshRendererAuthoring::HasComponent(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		return _world.HasComponent<MeshRenderer>(_entity);
	}

	bool MeshRendererAuthoring::AddComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (HasComponent(_world, _entity))
		{
			return false;
		}

		_world.AddComponent<MeshRenderer>(_entity, MeshRenderer{});
		return true;
	}

	bool MeshRendererAuthoring::RemoveComponent(cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return false;
		}

		return _world.RemoveComponent<MeshRenderer>(_entity);
	}

	std::vector<cp::AuthoringSectionDescriptor> MeshRendererAuthoring::BuildSections(const cp::ecs::World& _world, const cp::ecs::Entity& _entity) const
	{
		if (!HasComponent(_world, _entity))
		{
			return {};
		}

		const auto& renderer = _world.GetComponent<MeshRenderer>(_entity);
		return {
			cp::AuthoringSectionDescriptor{
				.id = "meshRenderer",
				.title = "MeshRenderer",
				.fields = {
					cp::AuthoringFieldDescriptor{
						.id = "visible",
						.label = "Visible",
						.valueType = cp::AuthoringValueType::Bool,
						.value = renderer.visible
					},
					cp::AuthoringFieldDescriptor{
						.id = "meshPath",
						.label = "Mesh Asset",
						.valueType = cp::AuthoringValueType::String,
						.inputType = cp::AuthoringInputType::FilePath,
						.value = renderer.meshId.IsValid()
							? GetRelativePath(renderer.meshId.GetAssetID()).string()
							: std::string{},
						.readOnly = false
					}
				}
			}
		};
	}

	bool MeshRendererAuthoring::ApplyValue(
		cp::ecs::World& _world,
		const cp::ecs::Entity& _entity,
		const std::string_view _fieldId,
		const cp::AuthoringValue& _value,
		std::string& _outError
	) const
	{
		if (!HasComponent(_world, _entity))
		{
			_outError = "MeshRenderer component is missing on selected entity.";
			return false;
		}

		if (_fieldId == "meshPath")
		{
			const std::string* stringValue = std::get_if<std::string>(&_value);
			if (stringValue == nullptr)
			{
				_outError = "Mesh asset path expects a string value.";
				return false;
			}

			auto& renderer = _world.GetComponent<MeshRenderer>(_entity);
			
			if (stringValue->empty())
			{
				renderer.meshId = AssetHandle<Mesh>{};
				return true;
			}

			auto& meshManager = cp::GetComponentRegistrationContext().GetAssetRegistry().Get<Mesh>();
			const auto meshPath = FindFileInParentTree(*stringValue);
			if (!std::filesystem::exists(meshPath))
			{
				_outError = "Mesh file not found: " + *stringValue;
				return false;
			}

			renderer.meshId = meshManager.Load(meshPath);
			if (!renderer.meshId.IsValid())
			{
				_outError = "Failed to load mesh: " + *stringValue;
				return false;
			}

			return true;
		}

		const bool* boolValue = std::get_if<bool>(&_value);
		if (boolValue == nullptr)
		{
			_outError = "MeshRenderer visible field expects a bool value.";
			return false;
		}

		auto& renderer = _world.GetComponent<MeshRenderer>(_entity);
		renderer.visible = *boolValue;
		return true;
	}
}
