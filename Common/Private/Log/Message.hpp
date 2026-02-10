#pragma once

#include <vector>
#include <memory>

#include "Components/ComponentTypeId.hpp"

namespace cp
{
	struct IMessageComponent;

	struct Message
	{
		std::vector<std::shared_ptr<IMessageComponent>> components;

		/**
		* @brief Add a component to the message.
		* 
		* @param component The component to add.
		* 
		* @return A new message containing the original components plus the new component.
		*/
		Message Then(const std::shared_ptr<IMessageComponent> component);

		/**
		* @brief Add a component of type T to the message, forwarding the arguments to its constructor.
		* 
		* @tparam T The type of the component to add, must derive from IMessageComponent.
		* 
		* @param args The arguments to forward to the component's constructor.
		* 
		* @return A new message containing the original components plus the new component of type T.
		*/
		template<MessageComponentType T, typename... Args>
		Message Then(Args&&... args)
		{
			return Then(std::make_shared<T>(std::forward<Args>(args)...));
		}

		/**
		* @brief Append another message's components to this message.
		* 
		* @param _message The message whose components to append.
		* 
		* @return A new message containing the combined components of this message and the provided message.
		*/
		Message Then(const Message& _message);

		/**
		* @brief Create a new message with a single component of type T, forwarding the arguments to its constructor.
		* 
		* @tparam T The type of the component to create, must derive from IMessageComponent.
		* 
		* @param args The arguments to forward to the component's constructor.
		* 
		* @return A new message containing a single component of type T.
		*/
		template<MessageComponentType T, typename... Args>
		static Message Create(Args&&... args)
		{
			Message message;
			message.Then<T>(std::forward<Args>(args)...);
			return message;
		}
	};
}