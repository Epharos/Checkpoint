#pragma once

class Serializer;

class Serializable
{
public:
	virtual void Serialize(Serializer& _serializer) const = 0;
	virtual void Deserialize(Serializer& _serializer) = 0;
};