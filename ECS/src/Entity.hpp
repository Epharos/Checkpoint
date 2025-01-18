#pragma once

#include "pch.hpp"

using ID = std::uint32_t;
using Version = std::uint8_t;

struct Entity
{
	ID id : 24;
	Version version : 8;

	bool operator==(const Entity& _other) const
	{
		return id == _other.id && version == _other.version;
	}

	bool operator!=(const Entity& _other) const
	{
		return !(*this == _other);
	}
};