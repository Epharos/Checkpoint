#pragma once

#include "pch.hpp"

using ID = std::uint32_t;

namespace ECS
{
	class Entity
	{
	private:
		const ID id;
		Signature<MAX_COMPONENTS> signature;

	public:
		Entity(ID id) : id(id) {}

		inline constexpr ID GetID() const { return id; }

		inline constexpr Signature<MAX_COMPONENTS> GetSignature() const { return signature; }

		inline constexpr Signature<MAX_COMPONENTS>& GetSignature() { return signature; }

		inline void SetSignature(Signature<MAX_COMPONENTS> signature) { this->signature = signature; }

		/*template<class T>
		void AddComponent();*/

		/*template<class T>
		void AddComponent(T component);*/

		/*template<class T>
		void RemoveComponent();*/
	};
}