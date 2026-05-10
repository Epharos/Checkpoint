#pragma once

#include <cstdint>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

namespace cp
{
	enum class AuthoringInputType : uint8_t
	{
		Default,
		FilePath
	};

	enum class AuthoringValueType : uint8_t
	{
		Bool,
		Int,
		Float,
		String,
		Vec3
	};

	struct AuthoringVec3
	{
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
	};

	using AuthoringValue = std::variant<bool, int64_t, double, std::string, AuthoringVec3>;

	using AuthoringParams = std::unordered_map<std::string, AuthoringValue>;

	struct AuthoringFieldDescriptor
	{
		std::string id;
		std::string label;
		AuthoringValueType valueType = AuthoringValueType::String;
		AuthoringInputType inputType = AuthoringInputType::Default;
		AuthoringValue value = std::string{};
		bool readOnly = false;
	};

	struct AuthoringSectionDescriptor
	{
		std::string id;
		std::string title;
		std::vector<AuthoringFieldDescriptor> fields;
	};
}
