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
		Default = 0,
		FilePath = 1,
		Color = 2,
	};

	enum class AuthoringValueType : uint8_t
	{
		Bool = 0,
		Int = 1,
		Float = 2,
		String = 3,
		Vec3 = 4,
		Vec2 = 5,
		Vec4 = 6,
	};

	struct AuthoringVec2
	{
		double x = 0.0;
		double y = 0.0;
	};

	struct AuthoringVec3
	{
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
	};

	struct AuthoringVec4
	{
		double x = 0.0;
		double y = 0.0;
		double z = 0.0;
		double w = 0.0;
	};

	using AuthoringValue = std::variant<bool, int64_t, double, std::string, AuthoringVec2, AuthoringVec3, AuthoringVec4>;

	using AuthoringParams = std::unordered_map<std::string, AuthoringValue>;

	struct AuthoringFieldDescriptor
	{
		std::string id;
		std::string label;
		AuthoringValueType valueType = AuthoringValueType::String;
		AuthoringInputType inputType = AuthoringInputType::Default;
		AuthoringValue value = std::string{};
		bool readOnly = false;

		std::vector<std::string> fileExtensions;
	};

	struct AuthoringSectionDescriptor
	{
		std::string id;
		std::string title;
		std::vector<AuthoringFieldDescriptor> fields;
	};
}
