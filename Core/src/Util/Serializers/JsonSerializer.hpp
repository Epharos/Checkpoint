#pragma once

#include "Serializer.hpp"

#include <nlohmann/json.hpp>

class JsonSerializer : public Serializer 
{
protected:

public:
	JsonSerializer() = default;
	~JsonSerializer() = default;

	void WriteString(const std::string& name, const std::string& value) override;
	void WriteInt(const std::string& name, int value) override;
	void WriteFloat(const std::string& name, float value) override;
	void WriteBool(const std::string& name, bool value) override;
	void WriteVector2(const std::string& name, const glm::vec2& value) override;
	void WriteVector3(const std::string& name, const glm::vec3& value) override;
	void WriteVector4(const std::string& name, const glm::vec4& value) override;
	void WriteQuaternion(const std::string& name, const glm::quat& value) override;
	void WriteColor(const std::string& name, const glm::vec4& value) override;

	std::string ReadString(const std::string& name, const std::string& defaultValue) override;
	int ReadInt(const std::string& name, int defaultValue) override;
	float ReadFloat(const std::string& name, float defaultValue) override;
	bool ReadBool(const std::string& name, bool defaultValue) override;
	glm::vec2 ReadVector2(const std::string& name, const glm::vec2& defaultValue) override;
	glm::vec3 ReadVector3(const std::string& name, const glm::vec3& defaultValue) override;
	glm::vec4 ReadVector4(const std::string& name, const glm::vec4& defaultValue) override;
	glm::quat ReadQuaternion(const std::string& name, const glm::quat& defaultValue) override;
	glm::vec4 ReadColor(const std::string& name, const glm::vec4& defaultValue) override;
};