#include "pch.hpp"
#include "JsonSerializer.hpp"

cp::JsonSerializer::JsonSerializer()
{
	objectStack.push_back(&data);
}

void cp::JsonSerializer::Write(const std::string& _path)
{
	std::ofstream file(_path);
	file << data.dump(4);
	file.close();
}

std::string cp::JsonSerializer::Read(const std::string& _path)
{
	size_t charCount = std::filesystem::file_size(_path);
	std::string content(charCount, '\0');
	std::ifstream file(_path);
	file.read(&content[0], charCount);
	data = json::parse(content);
	return content;
}

void cp::JsonSerializer::WriteString(const std::string& _name, const std::string& _value)
{
	(*objectStack.back())[_name] = _value;
}

void cp::JsonSerializer::WriteByte(const std::string& _name, const uint8_t& _value)
{
	(*objectStack.back())[_name] = static_cast<int>(_value);
}

void cp::JsonSerializer::WriteInt(const std::string& _name, int _value)
{
	(*objectStack.back())[_name] = _value;
}

void cp::JsonSerializer::WriteFloat(const std::string& _name, float _value)
{
	(*objectStack.back())[_name] = _value;
}

void cp::JsonSerializer::WriteBool(const std::string& _name, bool _value)
{
	(*objectStack.back())[_name] = _value;
}

void cp::JsonSerializer::WriteVector2(const std::string& _name, const glm::vec2& _value)
{
	(*objectStack.back())[_name]["x"] = _value.x;
	(*objectStack.back())[_name]["y"] = _value.y;
}

void cp::JsonSerializer::WriteVector3(const std::string& _name, const glm::vec3& _value)
{
	(*objectStack.back())[_name]["x"] = _value.x;
	(*objectStack.back())[_name]["y"] = _value.y;
	(*objectStack.back())[_name]["z"] = _value.z;
}

void cp::JsonSerializer::WriteVector4(const std::string& _name, const glm::vec4& _value)
{
	(*objectStack.back())[_name]["x"] = _value.x;
	(*objectStack.back())[_name]["y"] = _value.y;
	(*objectStack.back())[_name]["z"] = _value.z;
	(*objectStack.back())[_name]["w"] = _value.w;
}

void cp::JsonSerializer::WriteQuaternion(const std::string& _name, const glm::quat& _value)
{
	(*objectStack.back())[_name]["x"] = _value.x;
	(*objectStack.back())[_name]["y"] = _value.y;
	(*objectStack.back())[_name]["z"] = _value.z;
	(*objectStack.back())[_name]["w"] = _value.w;
}

void cp::JsonSerializer::WriteColor(const std::string& _name, const glm::vec4& _value)
{
	(*objectStack.back())[_name]["r"] = _value.r;
	(*objectStack.back())[_name]["g"] = _value.g;
	(*objectStack.back())[_name]["b"] = _value.b;
	(*objectStack.back())[_name]["a"] = _value.a;
}

void cp::JsonSerializer::BeginObjectWriting(const std::string& _name)
{
	objectStack.back()->operator[](_name) = json::object();
	objectStack.push_back(&(*objectStack.back())[_name]);
}

void cp::JsonSerializer::EndObject()
{
	if (objectStack.size() > 1)
		objectStack.pop_back();
}

void cp::JsonSerializer::WriteStringArray(const std::string& _name, const size_t& _size, const std::string* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteByteArray(const std::string& _name, const size_t& _size, const uint8_t* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(static_cast<int>(_values[i]));
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteIntArray(const std::string& _name, const size_t& _size, const int* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteFloatArray(const std::string& _name, const size_t& _size, const float* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteBoolArray(const std::string& _name, const size_t& _size, const bool* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		array.push_back(_values[i]);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteVector2Array(const std::string& _name, const size_t& _size, const glm::vec2* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec = json::object();
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		array.push_back(vec);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteVector3Array(const std::string& _name, const size_t& _size, const glm::vec3* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec = json::object();
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		array.push_back(vec);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteVector4Array(const std::string& _name, const size_t& _size, const glm::vec4* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec = json::object();
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		vec["w"] = _values[i].w;
		array.push_back(vec);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteQuaternionArray(const std::string& _name, const size_t& _size, const glm::quat* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec = json::object();
		vec["x"] = _values[i].x;
		vec["y"] = _values[i].y;
		vec["z"] = _values[i].z;
		vec["w"] = _values[i].w;
		array.push_back(vec);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::WriteColorArray(const std::string& _name, const size_t& _size, const glm::vec4* _values)
{
	json array = json::array();

	for (size_t i = 0; i < _size; i++)
	{
		json vec = json::object();
		vec["r"] = _values[i].r;
		vec["g"] = _values[i].g;
		vec["b"] = _values[i].b;
		vec["a"] = _values[i].a;
		array.push_back(vec);
	}

	(*objectStack.back())[_name] = array;
}

void cp::JsonSerializer::BeginObjectArrayWriting(const std::string& _name)
{
	objectStack.back()->operator[](_name) = json::array();
	objectStack.push_back(&(*objectStack.back())[_name]);
}

void cp::JsonSerializer::EndObjectArray()
{
	if (objectStack.size() > 1)
		objectStack.pop_back();
}

void cp::JsonSerializer::BeginObjectArrayElementWriting()
{
	objectStack.back()->push_back(json::object());
	objectStack.push_back(&objectStack.back()->back());
}

void cp::JsonSerializer::EndObjectArrayElement()
{
	if (objectStack.size() > 1)
		objectStack.pop_back();
}

std::string cp::JsonSerializer::ReadString(const std::string& _name, const std::string& _defaultValue)
{
	return objectStack.back()->contains(_name) ? objectStack.back()->operator[](_name).get<std::string>() : _defaultValue;
}

uint8_t cp::JsonSerializer::ReadByte(const std::string& _name, const uint8_t& _defaultValue)
{
	return objectStack.back()->contains(_name) ? static_cast<uint8_t>(objectStack.back()->operator[](_name).get<int>()) : _defaultValue;
}

int cp::JsonSerializer::ReadInt(const std::string& _name, int _defaultValue)
{
	return objectStack.back()->contains(_name) ? objectStack.back()->operator[](_name).get<int>() : _defaultValue;
}

float cp::JsonSerializer::ReadFloat(const std::string& _name, float _defaultValue)
{
	return objectStack.back()->contains(_name) ? objectStack.back()->operator[](_name).get<float>() : _defaultValue;
}

bool cp::JsonSerializer::ReadBool(const std::string& _name, bool _defaultValue)
{
	return objectStack.back()->contains(_name) ? objectStack.back()->operator[](_name).get<bool>() : _defaultValue;
}

glm::vec2 cp::JsonSerializer::ReadVector2(const std::string& _name, const glm::vec2& _defaultValue)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_object())
	{
		glm::vec2 vec;
		vec.x = objectStack.back()->operator[](_name)["x"].get<float>();
		vec.y = objectStack.back()->operator[](_name)["y"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec3 cp::JsonSerializer::ReadVector3(const std::string& _name, const glm::vec3& _defaultValue)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_object())
	{
		glm::vec3 vec;
		vec.x = objectStack.back()->operator[](_name)["x"].get<float>();
		vec.y = objectStack.back()->operator[](_name)["y"].get<float>();
		vec.z = objectStack.back()->operator[](_name)["z"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec4 cp::JsonSerializer::ReadVector4(const std::string& _name, const glm::vec4& _defaultValue)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_object())
	{
		glm::vec4 vec;
		vec.x = objectStack.back()->operator[](_name)["x"].get<float>();
		vec.y = objectStack.back()->operator[](_name)["y"].get<float>();
		vec.z = objectStack.back()->operator[](_name)["z"].get<float>();
		vec.w = objectStack.back()->operator[](_name)["w"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::quat cp::JsonSerializer::ReadQuaternion(const std::string& _name, const glm::quat& _defaultValue)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_object())
	{
		glm::quat vec;
		vec.x = objectStack.back()->operator[](_name)["x"].get<float>();
		vec.y = objectStack.back()->operator[](_name)["y"].get<float>();
		vec.z = objectStack.back()->operator[](_name)["z"].get<float>();
		vec.w = objectStack.back()->operator[](_name)["w"].get<float>();
		return vec;
	}

	return _defaultValue;
}

glm::vec4 cp::JsonSerializer::ReadColor(const std::string& _name, const glm::vec4& _defaultValue)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_object())
	{
		glm::vec4 vec;
		vec.r = objectStack.back()->operator[](_name)["r"].get<float>();
		vec.g = objectStack.back()->operator[](_name)["g"].get<float>();
		vec.b = objectStack.back()->operator[](_name)["b"].get<float>();
		vec.a = objectStack.back()->operator[](_name)["a"].get<float>();
		return vec;
	}

	return _defaultValue;
}

bool cp::JsonSerializer::BeginObjectReading(const std::string& _name)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->is_object())
	{
		objectStack.push_back(&(*objectStack.back())[_name]);
		return true;
	}

	return false;
}

std::tuple<size_t, std::string*> cp::JsonSerializer::ReadStringArray(const std::string& _name)
{
	size_t size = data[_name].size();
	std::string* array = new std::string[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<std::string>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, uint8_t*> cp::JsonSerializer::ReadByteArray(const std::string& _name)
{
	if (!objectStack.back()->contains(_name) || !objectStack.back()->operator[](_name).is_array())
	{
		LOG_ERROR(MF("Byte array ", _name, " not found or is not an array"));
		return std::make_tuple(0, nullptr);
	}

	objectStack.push_back(&(*objectStack.back())[_name]);

	size_t size = objectStack.back()->size();
	uint8_t* array = new uint8_t[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = static_cast<uint8_t>(objectStack.back()->at(i).get<int>());
	}

	if (objectStack.size() > 1)
		objectStack.pop_back();

	return std::make_tuple(size, array);
}

std::tuple<size_t, int*> cp::JsonSerializer::ReadIntArray(const std::string& _name)
{
	size_t size = data[_name].size();
	int* array = new int[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<int>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, float*> cp::JsonSerializer::ReadFloatArray(const std::string& _name)
{
	size_t size = data[_name].size();
	float* array = new float[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<float>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, bool*> cp::JsonSerializer::ReadBoolArray(const std::string& _name)
{
	size_t size = data[_name].size();
	bool* array = new bool[size];

	for (size_t i = 0; i < size; i++)
	{
		array[i] = data[_name][i].get<bool>();
	}

	return std::make_tuple(size, array);
}

std::tuple<size_t, glm::vec2*> cp::JsonSerializer::ReadVector2Array(const std::string& _name)
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

std::tuple<size_t, glm::vec3*> cp::JsonSerializer::ReadVector3Array(const std::string& _name)
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

std::tuple<size_t, glm::vec4*> cp::JsonSerializer::ReadVector4Array(const std::string& _name)
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

std::tuple<size_t, glm::quat*> cp::JsonSerializer::ReadQuaternionArray(const std::string& _name)
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

std::tuple<size_t, glm::vec4*> cp::JsonSerializer::ReadColorArray(const std::string& _name)
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

size_t cp::JsonSerializer::BeginObjectArrayReading(const std::string& _name)
{
	if (objectStack.back()->contains(_name) && objectStack.back()->operator[](_name).is_array())
	{
		objectStack.push_back(&(*objectStack.back())[_name]);
		return objectStack.back()->size();
	}

	return 0;
}

bool cp::JsonSerializer::BeginObjectArrayElementReading(const uint64_t _index)
{
	if (objectStack.back()->size() > _index && objectStack.back()->at(_index).is_object())
	{
		objectStack.push_back(&objectStack.back()->at(_index));
		return true;
	}

	return false;
}
