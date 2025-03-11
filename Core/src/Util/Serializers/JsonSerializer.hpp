#pragma once

#include "Serializer.hpp"
#include <fstream>

#include <nlohmann/json.hpp>

#include "Serializable.hpp"

using json = nlohmann::json;

class JsonSerializer : public Serializer 
{
protected:
	json data;

	inline json GetRawData() { return data; }
	inline void SetRawData(json& _data) { data = _data; }

public:
	JsonSerializer() = default;
	~JsonSerializer() = default;

	void Write(const std::string& _path) override;
	std::string Read(const std::string& _path) override;

	void WriteString(const std::string& _name, const std::string& _value) override;
	void WriteInt(const std::string& _name, int _value) override;
	void WriteFloat(const std::string& _name, float _value) override;
	void WriteBool(const std::string& _name, bool _value) override;
	void WriteVector2(const std::string& _name, const glm::vec2& _value) override;
	void WriteVector3(const std::string& _name, const glm::vec3& _value) override;
	void WriteVector4(const std::string& _name, const glm::vec4& _value) override;
	void WriteQuaternion(const std::string& _name, const glm::quat& _value) override;
	void WriteColor(const std::string& _name, const glm::vec4& _value) override;
	void WriteObject(const std::string& _name, const Serializable*& _object) override;
	void WriteObject(const std::string& _name, std::function<void(Serializer&)> _serializeFunction) override;
	void WriteStringArray(const std::string& _name, const size_t& _size, const std::string* _values) override;
	void WriteIntArray(const std::string& _name, const size_t& _size, const int* _values) override;
	void WriteFloatArray(const std::string& _name, const size_t& _size, const float* _values) override;
	void WriteBoolArray(const std::string& _name, const size_t& _size, const bool* _values) override;
	void WriteVector2Array(const std::string& _name, const size_t& _size, const glm::vec2* _values) override;
	void WriteVector3Array(const std::string& _name, const size_t& _size, const glm::vec3* _values) override;
	void WriteVector4Array(const std::string& _name, const size_t& _size, const glm::vec4* _values) override;
	void WriteQuaternionArray(const std::string& _name, const size_t& _size, const glm::quat* _values) override;
	void WriteColorArray(const std::string& _name, const size_t& _size, const glm::vec4* _values) override;
	void WriteObjectArray(const std::string& _name, const size_t& _size, const Serializable**& _objects) override;
	void WriteObjectArray(const std::string& _name, const size_t& _size, const void* _objects, std::function<void(const void*, Serializer&)> _serializeFunction) override;

	std::string ReadString(const std::string& _name, const std::string& _defaultValue) override;
	int ReadInt(const std::string& _name, int _defaultValue) override;
	float ReadFloat(const std::string& _name, float _defaultValue) override;
	bool ReadBool(const std::string& _name, bool _defaultValue) override;
	glm::vec2 ReadVector2(const std::string& _name, const glm::vec2& _defaultValue) override;
	glm::vec3 ReadVector3(const std::string& _name, const glm::vec3& _defaultValue) override;
	glm::vec4 ReadVector4(const std::string& _name, const glm::vec4& _defaultValue) override;
	glm::quat ReadQuaternion(const std::string& _name, const glm::quat& _defaultValue) override;
	glm::vec4 ReadColor(const std::string& _name, const glm::vec4& _defaultValue) override;
	void ReadObject(const std::string& _name, Serializable*& _object) override;
	void ReadObject(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction) override;
	std::tuple<size_t, std::string*> ReadStringArray(const std::string& _name) override;
	std::tuple<size_t, int*> ReadIntArray(const std::string& _name) override;
	std::tuple<size_t, float*> ReadFloatArray(const std::string& _name) override;
	std::tuple<size_t, bool*> ReadBoolArray(const std::string& _name) override;
	std::tuple<size_t, glm::vec2*> ReadVector2Array(const std::string& _name) override;
	std::tuple<size_t, glm::vec3*> ReadVector3Array(const std::string& _name) override;
	std::tuple<size_t, glm::vec4*> ReadVector4Array(const std::string& _name) override;
	std::tuple<size_t, glm::quat*> ReadQuaternionArray(const std::string& _name) override;
	std::tuple<size_t, glm::vec4*> ReadColorArray(const std::string& _name) override;
	size_t ReadObjectArray(const std::string& _name, Serializable**& _objects) override;
	size_t ReadObjectArray(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction) override;
};