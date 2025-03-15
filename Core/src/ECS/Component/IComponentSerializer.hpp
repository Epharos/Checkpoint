#pragma once

#include "../../Util/Serializers/Serializable.hpp"
#include "../../Util/Serializers/ISerializer.hpp"

namespace cp
{
	class ComponentSerializerBase : public ISerializable
	{

	};

	class IComponentSerializer : public ComponentSerializerBase
	{
	protected:
		IComponentBase& component;

	public:
		IComponentSerializer(IComponentBase& _component) : component(_component) {}
	};
};