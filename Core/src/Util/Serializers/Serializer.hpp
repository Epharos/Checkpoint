#pragma once

#include "../../pch.hpp"

class Serializer 
{
public:
	virtual void WriteString(const std::string& name, const std::string& value) = 0;
	virtual void WriteInt(const std::string& name, int value) = 0;
	virtual void WriteFloat(const std::string& name, float value) = 0;
	virtual void WriteBool(const std::string& name, bool value) = 0;
	virtual void WriteVector2(const std::string& name, const glm::vec2& value) = 0;
	virtual void WriteVector3(const std::string& name, const glm::vec3& value) = 0;
	virtual void WriteVector4(const std::string& name, const glm::vec4& value) = 0;
	virtual void WriteQuaternion(const std::string& name, const glm::quat& value) = 0;
	virtual void WriteColor(const std::string& name, const glm::vec4& value) = 0;

	virtual std::string ReadString(const std::string& name, const std::string& defaultValue) = 0;
	virtual int ReadInt(const std::string& name, int defaultValue) = 0;
	virtual float ReadFloat(const std::string& name, float defaultValue) = 0;
	virtual bool ReadBool(const std::string& name, bool defaultValue) = 0;
	virtual glm::vec2 ReadVector2(const std::string& name, const glm::vec2& defaultValue) = 0;
	virtual glm::vec3 ReadVector3(const std::string& name, const glm::vec3& defaultValue) = 0;
	virtual glm::vec4 ReadVector4(const std::string& name, const glm::vec4& defaultValue) = 0;
	virtual glm::quat ReadQuaternion(const std::string& name, const glm::quat& defaultValue) = 0;
	virtual glm::vec4 ReadColor(const std::string& name, const glm::vec4& defaultValue) = 0;
};