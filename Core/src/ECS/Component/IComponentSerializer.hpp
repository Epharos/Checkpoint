#pragma once

#include "../../Util/Serializers/Serializable.hpp"
#include "../../Util/Serializers/Serializer.hpp"

class ComponentSerializerBase : public Serializable
{

};

template<class T>
class IComponentSerializer : public ComponentSerializerBase
{
protected:
	T& component;

public:
	IComponentSerializer(T& _component) : component(_component) {}
};