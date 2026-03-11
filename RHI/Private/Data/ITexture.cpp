#include "ITexture.hpp"

namespace cp
{
	ITexture::ITexture(const TextureInfo& _info, TextureLayout _initialLayout)
		: info(_info), layout(_initialLayout)
	{
	}

	void ITexture::UpdateLayout(const TextureLayout _layout)
	{
		layout = _layout;
	}
}
