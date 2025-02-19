#include "pch.hpp"
#include "PlatformQt.hpp"

#include "../VulkanContext.hpp"

void Context::PlatformQt::Initialize(VulkanContextInfo _context, vk::Extent2D _extent)
{
	throw std::runtime_error("Initialize(VulkanContextInfo, vk::Extent2D) is deprecated with Qt usage. Use Initialize(QVulkanWindow*) instead");
}

void Context::PlatformQt::Initialize(QWindow* _window)
{
	window = _window;
}

bool Context::PlatformQt::ShouldClose() const
{
	return false;
}

void Context::PlatformQt::PollEvents() const
{

}

void Context::PlatformQt::CleanUp() const
{

}

void Context::PlatformQt::SetTitle(const std::string& _title) const
{
	window->setTitle(QString::fromStdString(_title));
}

vk::Extent2D Context::PlatformQt::GetExtent() const
{
	return vk::Extent2D(window->size().width(), window->size().height());
}
