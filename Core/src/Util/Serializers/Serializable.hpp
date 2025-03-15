#pragma once

namespace cp
{
	class ISerializer;

	class ISerializable
	{
	public:
		virtual void Serialize(ISerializer& _serializer) const = 0;
		virtual void Deserialize(ISerializer& _serializer) = 0;
	};
};