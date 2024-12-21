#pragma once

#include "pch.hpp"

using ID = std::uint32_t;
using Version = std::uint8_t;

struct Entity
{
	ID id : 24;
	Version version : 8;
};