#pragma once

#include <concepts>
#include <cstdint>

namespace cp
{
	struct IMessageComponent;

	template<typename T>
	concept MessageComponentType = std::derived_from<T, IMessageComponent>;

	using MessageComponentTypeId = uint32_t;

	/**
	* @brief Generates a unique MessageComponentTypeId for each component type.
	* 
	* @return MessageComponentTypeId A unique identifier for a component type.
	*/
	inline MessageComponentTypeId GetUniqueMessageComponentTypeId()
	{
		static MessageComponentTypeId lastId = 0;
		return lastId++;
	}

	/**
	* @brief Retrieves the unique MessageComponentTypeId for a specific component type T.
	* 
	* @tparam T The component type for which to retrieve the type ID. Must satisfy the MessageComponentType concept.
	* 
	* @return MessageComponentTypeId The unique identifier associated with the component type T.
	*/
	template<MessageComponentType T>
	inline MessageComponentTypeId GetMessageComponentTypeId()
	{
		static MessageComponentTypeId typeId = GetUniqueMessageComponentTypeId();
		return typeId;
	}
}