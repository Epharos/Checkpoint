#pragma once

#include <QtCore/qjsonobject.h>

class ComponentSerializerBase
{
public:
	virtual QJsonObject Serialize(void* _component) = 0;
	virtual void Deserialize(const QJsonObject& _data, void* _component) = 0;
};

template<class T>
class IComponentSerializer : public ComponentSerializerBase
{
	//TODO : Not use Qt Json for serialization

public:
	virtual QJsonObject Serialize(const T& _component) = 0;
	virtual void Deserialize(const QJsonObject& _data, T& _component) = 0;

	QJsonObject Serialize(void* _component) override
	{
		return Serialize(*static_cast<T*>(_component));
	}

	void Deserialize(const QJsonObject& _data, void* _component) override
	{
		Deserialize(_data, *static_cast<T*>(_component));
	}
};