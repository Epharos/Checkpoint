#include "pch.hpp"
#include "JsonSerializer.hpp"

void JsonSerializer::Write(const std::string& _path)
{
	std::ofstream file(_path);
	file << data.dump(4);
	file.close();
}

std::string JsonSerializer::Read(const std::string& _path)
{
	size_t charCount = std::filesystem::file_size(_path);
	std::string content(charCount, '\0');
	std::ifstream file(_path);
	file.read(&content[0], charCount);
	data = json::parse(content);
	return content;
}

void JsonSerializer::WriteString(const std::string& _name, const std::string& _value)
{
	data[_name] = _value;
}

void JsonSerializer::WriteInt(const std::string& _name, int _value)
{
	data[_name] = _value;
}

void JsonSerializer::WriteFloat(const std::string& _name, float _value)
{
	data[_name] = _value;
}

void JsonSerializer::WriteBool(const std::string& _name, bool _value)
{
	data[_name] = _value;
}

void JsonSerializer::WriteVector2(const std::string& _name, const glm::vec2& _value)
{
	data[_name]["x"] = _value.x;
	data[_name]["y"] = _value.y;
}

void JsonSerializer::WriteVector3(const std::string& _name, const glm::vec3& _value)
{
	data[_name]["x"] = _value.x;
	data[_name]["y"] = _value.y;
	data[_name]["z"] = _value.z;
}

void JsonSerializer::WriteVector4(const std::string& _name, const glm::vec4& _value)
{
	data[_name]["x"] = _value.x;
	data[_name]["y"] = _value.y;
	data[_name]["z"] = _value.z;
	data[_name]["w"] = _value.w;
}

void JsonSerializer::WriteQuaternion(const std::string& _name, const glm::quat& _value)
{
	data[_name]["x"] = _value.x;
	data[_name]["y"] = _value.y;
	data[_name]["z"] = _value.z;
	data[_name]["w"] = _value.w;
}

void JsonSerializer::WriteColor(const std::string& _name, const glm::vec4& _value)
{
	data[_name]["r"] = _value.r;
	data[_name]["g"] = _value.g;
	data[_name]["b"] = _value.b;
	data[_name]["a"] = _value.a;
}

void JsonSerializer::WriteObject(const std::string& _name, const Serializable*& _object)
{
	JsonSerializer serializer;
	_object->Serialize(serializer);
	data[_name] = serializer.GetRawData();
}

void JsonSerializer::WriteObject(const std::string& _name, std::function<void(Serializer&)> _serializeFunction)
{
	JsonSerializer serializer;
	_serializeFunction(serializer);
	data[_name] = serializer.GetRawData();
}

void JsonSerializer::WriteStringArray(const std::string& _name, const size_t& _size, const std::string* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	data[_name] = array;
}

void JsonSerializer::WriteIntArray(const std::string& _name, const size_t& _size, const int* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	data[_name] = array;
}

void JsonSerializer::WriteFloatArray(const std::string& _name, const size_t& _size, const float* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	data[_name] = array;
}

void JsonSerializer::WriteBoolArray(const std::string& _name, const size_t& _size, const bool* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	data[_name] = array;
}

void JsonSerializer::WriteVector2Array(const std::string& _name, const size_t& _size, const glm::vec2* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec;
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		array.push_back(vec);
	}

	data[_name] = array;
}

void JsonSerializer::WriteVector3Array(const std::string& _name, const size_t& _size, const glm::vec3* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec;
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		array.push_back(vec);
	}

	data[_name] = array;
}

void JsonSerializer::WriteVector4Array(const std::string& _name, const size_t& _size, const glm::vec4* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec;
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		vec["w"] = _values[i].w;
		array.push_back(vec);
	}

	data[_name] = array;
}

void JsonSerializer::WriteQuaternionArray(const std::string& _name, const size_t& _size, const glm::quat* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec;
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		vec["w"] = _values[i].w;
		array.push_back(vec);
	}

	data[_name] = array;
}

void JsonSerializer::WriteColorArray(const std::string& _name, const size_t& _size, const glm::vec4* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec;
		vec["r"] = _values[i].r;
		vec["g"] = _values[i].g;
		vec["b"] = _values[i].b;
		vec["a"] = _values[i].a;
		array.push_back(vec);
	}

	data[_name] = array;
}

void JsonSerializer::WriteObjectArray(const std::string& _name, const size_t& _size, const Serializable**& _objects)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		JsonSerializer serializer;
		_objects[i]->Serialize(serializer);
		array.push_back(serializer.GetRawData());
	}

	data[_name] = array;
}

void JsonSerializer::WriteObjectArray(const std::string& _name, const size_t& _size, const void** _objects, std::function<void(const void*, Serializer&)> _serializeFunction)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		JsonSerializer serializer;
		_serializeFunction(_objects[i], serializer);
		array.push_back(serializer.GetRawData());
	}

	data[_name] = array;
}

std::string JsonSerializer::ReadString(const std::string& _name, const std::string& _defaultValue)
{
	return data.contains(_name) ? data[_name].get<std::string>() : _defaultValue;
}

int JsonSerializer::ReadInt(const std::string& _name, int _defaultValue)
{
	return data.contains(_name) ? data[_name].get<int>() : _defaultValue;
}

float JsonSerializer::ReadFloat(const std::string& _name, float _defaultValue)
{
	return data.contains(_name) ? data[_name].get<float>() : _defaultValue;
}

bool JsonSerializer::ReadBool(const std::string& _name, bool _defaultValue)
{
	return data.contains(_name) ? data[_name].get<bool>() : _defaultValue;
}

glm::vec2 JsonSerializer::ReadVector2(const std::string& _name, const glm::vec2& _defaultValue)
{
	if (data.contains(_name))
	{
		glm::vec2 vec;
		vec.x = data[_name]["x"].get<float>();
		vec.y = data[_name]["y"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec3 JsonSerializer::ReadVector3(const std::string& _name, const glm::vec3& _defaultValue)
{
	if (data.contains(_name))
	{
		glm::vec3 vec;
		vec.x = data[_name]["x"].get<float>();
		vec.y = data[_name]["y"].get<float>();
		vec.z = data[_name]["z"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec4 JsonSerializer::ReadVector4(const std::string& _name, const glm::vec4& _defaultValue)
{
	if (data.contains(_name))
	{
		glm::vec4 vec;
		vec.x = data[_name]["x"].get<float>();
		vec.y = data[_name]["y"].get<float>();
		vec.z = data[_name]["z"].get<float>();
		vec.w = data[_name]["w"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::quat JsonSerializer::ReadQuaternion(const std::string& _name, const glm::quat& _defaultValue)
{
	if (data.contains(_name))
	{
		glm::quat vec;
		vec.x = data[_name]["x"].get<float>();
		vec.y = data[_name]["y"].get<float>();
		vec.z = data[_name]["z"].get<float>();
		vec.w = data[_name]["w"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec4 JsonSerializer::ReadColor(const std::string& _name, const glm::vec4& _defaultValue)
{
	if (data.contains(_name))
	{
		glm::vec4 vec;
		vec.r = data[_name]["r"].get<float>();
		vec.g = data[_name]["g"].get<float>();
		vec.b = data[_name]["b"].get<float>();
		vec.a = data[_name]["a"].get<float>();
		return vec;
	}

	return _defaultValue;
}

void JsonSerializer::ReadObject(const std::string& _name, Serializable*& _object)
{
	JsonSerializer serializer;
	serializer.SetRawData(data[_name]);
	_object->Deserialize(serializer);
}

void JsonSerializer::ReadObject(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction)
{
	JsonSerializer serializer;
	serializer.SetRawData(data[_name]);
	_deserializeFunction(serializer);
}

std::tuple<size_t, std::string*> JsonSerializer::ReadStringArray(const std::string& _name)
{
	size_t size = data[_name].size();
	std::string* array = new std::string[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<std::string>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, int*> JsonSerializer::ReadIntArray(const std::string& _name)
{
	size_t size = data[_name].size();
	int* array = new int[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<int>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, float*> JsonSerializer::ReadFloatArray(const std::string& _name)
{
	size_t size = data[_name].size();
	float* array = new float[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, bool*> JsonSerializer::ReadBoolArray(const std::string& _name)
{
	size_t size = data[_name].size();
	bool* array = new bool[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<bool>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::vec2*> JsonSerializer::ReadVector2Array(const std::string& _name)
{
	size_t size = data[_name].size();
	glm::vec2* array = new glm::vec2[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i].x = data[_name][i]["x"].get<float>();
		array[i].y = data[_name][i]["y"].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::vec3*> JsonSerializer::ReadVector3Array(const std::string& _name)
{
	size_t size = data[_name].size();
	glm::vec3* array = new glm::vec3[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i].x = data[_name][i]["x"].get<float>();
		array[i].y = data[_name][i]["y"].get<float>();
		array[i].z = data[_name][i]["z"].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::vec4*> JsonSerializer::ReadVector4Array(const std::string& _name)
{
	size_t size = data[_name].size();
	glm::vec4* array = new glm::vec4[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i].x = data[_name][i]["x"].get<float>();
		array[i].y = data[_name][i]["y"].get<float>();
		array[i].z = data[_name][i]["z"].get<float>();
		array[i].w = data[_name][i]["w"].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::quat*> JsonSerializer::ReadQuaternionArray(const std::string& _name)
{
	size_t size = data[_name].size();
	glm::quat* array = new glm::quat[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i].x = data[_name][i]["x"].get<float>();
		array[i].y = data[_name][i]["y"].get<float>();
		array[i].z = data[_name][i]["z"].get<float>();
		array[i].w = data[_name][i]["w"].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::vec4*> JsonSerializer::ReadColorArray(const std::string& _name)
{
	size_t size = data[_name].size();
	glm::vec4* array = new glm::vec4[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i].r = data[_name][i]["r"].get<float>();
		array[i].g = data[_name][i]["g"].get<float>();
		array[i].b = data[_name][i]["b"].get<float>();
		array[i].a = data[_name][i]["a"].get<float>();
	}

	return std::make_tuple(size, array);
}

size_t JsonSerializer::ReadObjectArray(const std::string& _name, Serializable**& _objects)
{
	if (!data.contains(_name)) return static_cast<size_t>(0);

	size_t size = data[_name].size();

	for (size_t i = 0; i < size; i++)
	{
		JsonSerializer serializer;
		serializer.SetRawData(data[_name][i]);
		_objects[i]->Deserialize(serializer);
	}

	return size;
}

size_t JsonSerializer::ReadObjectArray(const std::string& _name, std::function<void(Serializer&)> _deserializeFunction)
{
	if (!data.contains(_name)) return static_cast<size_t>(0);

	size_t size = data[_name].size();

	for (size_t i = 0; i < size; i++)
	{
		JsonSerializer serializer;
		serializer.SetRawData(data[_name][i]);
		_deserializeFunction(serializer);
	}

	return size;
}
