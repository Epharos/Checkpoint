#include "Message.hpp"

namespace cp
{
	Message Message::Then(const std::shared_ptr<IMessageComponent> component)
	{
		components.push_back(component);
		return *this;
	}

	Message Message::Then(const Message& _message)
	{
		for (auto& component : _message.components)
		{
			Then(component);
		}

		return *this;
	}
}