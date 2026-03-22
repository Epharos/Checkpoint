#include "ITexture.hpp"

namespace cp
{
	ITexture::ITexture(const TextureInfo& _info)
		: info(_info)
	{
	}

	void ITexture::UpdateLayout(const TextureLayout _layout)
	{
		layout = _layout;
	}
}
