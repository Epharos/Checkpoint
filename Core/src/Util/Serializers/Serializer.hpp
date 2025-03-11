#pragma once

#include "../../pch.hpp"

#include "Serializable.hpp"

class Serializer 
{
public:
	virtual void Write(const std::string& _path) = 0;
	virtual std::string Read(const std::string& _path) = 0;

	virtual void WriteString(const std::string& _name, const std::string& _value) = 0;
	virtual void WriteInt(const std::string& _name, int _value) = 0;
	virtual void WriteFloat(const std::string& _name, float _value) = 0;
	virtual void WriteBool(const std::string& _name, bool _value) = 0;
	virtual void WriteVector2(const std::string& _name, const glm::vec2& _value) = 0;
	virtual void WriteVector3(const std::string& _name, const glm::vec3& _value) = 0;
	virtual void WriteVector4(const std::string& _name, const glm::vec4& _value) = 0;
	virtual void WriteQuaternion(const std::string& _name, const glm::quat& _value) = 0;
	virtual void WriteColor(const std::string& _name, const glm::vec4& _value) = 0;
	virtual void WriteObject(const std::string& _name, const Serializable*& _object) = 0;
	virtual void WriteObject(const std::string& _name, std::function<void(Serializer&)> _serializeFunction) = 0;
	virtual void WriteStringArray(const std::string& _name, const size_t& _size, const std::string* _values) = 0;
	virtual void WriteIntArray(const std::string& _name, const size_t& _size, const int* _values) = 0;
	virtual void WriteFloatArray(const std::string& _name, const size_t& _size, const float* _values) = 0;
	virtual void WriteBoolArray(const std::string& _name, const size_t& _size, const bool* _values) = 0;
	virtual void WriteVector2Array(const std::string& _name, const size_t& _size, const glm::vec2* _values) = 0;
	virtual void WriteVector3Array(const std::string& _name, const size_t& _size, const glm::vec3* _values) = 0;
	virtual void WriteVector4Array(const std::string& _name, const size_t& _size, const glm::vec4* _values) = 0;
	virtual void WriteQuaternionArray(const std::string& _name, const size_t& _size, const glm::quat* _values) = 0;
	virtual void WriteColorArray(const std::string& _name, const size_t& _size, const glm::vec4* _values) = 0;
	virtual void WriteObjectArray(const std::string& _name, const size_t& _size, const Serializable**& _objects) = 0;
	virtual void WriteObjectArray(const std::string& _name, const size_t& _size, const void* _objects, std::function<void(const void*, Serializer&)> _serializeFunction) = 0;

	virtual std::string ReadString(const std::string& _name, const std::string& _defaultValue) = 0;
	virtual int ReadInt(const std::string& _name, int _defaultValue) = 0;
	virtual float ReadFloat(const std::string& _name, float _defaultValue) = 0;
	virtual bool ReadBool(const std::string& _name, bool _defaultValue) = 0;
	virtual glm::vec2 ReadVector2(const std::string& _name, const glm::vec2& _defaultValue) = 0;
	virtual glm::vec3 ReadVector3(const std::string& _name, const glm::vec3& _defaultValue) = 0;
	virtual glm::vec4 ReadVector4(const std::string& _name, const glm::vec4& _defaultValue) = 0;
	virtual glm::quat ReadQuaternion(const std::string& _name, const glm::quat& _defaultValue) = 0;
	virtual glm::vec4 ReadColor(const std::string& _name, const glm::vec4& _defaultValue) = 0;
	virtual void ReadObject(const std::string& _name, Serializable*& _object) = 0;
	virtual void ReadObject(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction) = 0;
	virtual std::tuple<size_t, std::string*> ReadStringArray(const std::string& _name) = 0;
	virtual std::tuple<size_t, int*> ReadIntArray(const std::string& _name) = 0;
	virtual std::tuple<size_t, float*> ReadFloatArray(const std::string& _name) = 0;
	virtual std::tuple<size_t, bool*> ReadBoolArray(const std::string& _name) = 0;
	virtual std::tuple<size_t, glm::vec2*> ReadVector2Array(const std::string& _name) = 0;
	virtual std::tuple<size_t, glm::vec3*> ReadVector3Array(const std::string& _name) = 0;
	virtual std::tuple<size_t, glm::vec4*> ReadVector4Array(const std::string& _name) = 0;
	virtual std::tuple<size_t, glm::quat*> ReadQuaternionArray(const std::string& _name) = 0;
	virtual std::tuple<size_t, glm::vec4*> ReadColorArray(const std::string& _name) = 0;
	virtual size_t ReadObjectArray(const std::string& _name, Serializable**& _objects) = 0;
	virtual size_t ReadObjectArray(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction) = 0;
};