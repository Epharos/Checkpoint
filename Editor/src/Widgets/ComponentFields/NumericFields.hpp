#pragma once

#include "../../pch.hpp"
#include "Numeric.hpp"

class UInt8 : public Numeric<uint8_t>
{
	Q_OBJECT

public:
	UInt8(uint8_t* _val, const std::string& _fieldName, QWidget* parent = nullptr) : Numeric(_val, _fieldName, parent)
	{
	}
};

class UInt16 : public Numeric<uint16_t>
{
	Q_OBJECT
public:

	UInt16(uint16_t* _val, const std::string& _fieldName, QWidget* parent = nullptr) : Numeric(_val, _fieldName, parent)
	{
	}
};

class UInt32 : public Numeric<uint32_t>
{
	Q_OBJECT
public:

	UInt32(uint32_t* _val, const std::string& _fieldName, QWidget* parent = nullptr) : Numeric(_val, _fieldName, parent)
	{
	}
};

class UInt64 : public Numeric<uint64_t>
{
	Q_OBJECT

public:
	UInt64(uint64_t* _val, const std::string& _fieldName, QWidget* parent = nullptr) : Numeric(_val, _fieldName, parent)
	{
	}
};