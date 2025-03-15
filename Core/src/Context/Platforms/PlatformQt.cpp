#include "pch.hpp"
#include "PlatformQt.hpp"

#include "../VulkanContext.hpp"

void cp::PlatformQt::Initialize(VulkanContextInfo _context, vk::Extent2D _extent)
{
	throw std::runtime_error("Initialize(VulkanContextInfo, vk::Extent2D) is deprecated with Qt usage. Use Initialize(QVulkanWindow*) instead");
}

void cp::PlatformQt::Initialize(QWindow* _window)
{
	window = _window;
}

bool cp::PlatformQt::ShouldClose() const
{
	return false;
}

void cp::PlatformQt::PollEvents() const
{

}

void cp::PlatformQt::CleanUp() const
{

}

void cp::PlatformQt::SetTitle(const std::string& _title) const
{
	window->setTitle(QString::fromStdString(_title));
}

vk::Extent2D cp::PlatformQt::GetExtent() const
{
	return vk::Extent2D(window->size().width(), window->size().height());
}
