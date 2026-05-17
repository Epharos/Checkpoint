#include "IWindow.hpp"

namespace cp
{
	IWindow::IWindow(const WindowInfo& _info)
	{
	}

	void IWindow::SetResizeCallback(std::function<void(Extent2D<int>)> _callback)
	{
		resizeCallback = std::move(_callback);
	}

	void IWindow::InvokeResizeCallback(Extent2D<int> _newExtent) const
	{
		if (resizeCallback)
		{
			resizeCallback(_newExtent);
		}
	}
}