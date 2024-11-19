#pragma once

#include "../pch.hpp"

typedef GLFWwindow* Window; //future proof, in case we change the windowing library

namespace Context
{
	struct VulkanContextInfo;

	class Platform
	{
	private:
		Window window;
	public:
		void Initialize(VulkanContextInfo _context);
		bool ShouldClose() const;
		void PollEvents() const;
		void CleanUp() const;

		inline constexpr Window GetWindow() const { return window; }
	};
}