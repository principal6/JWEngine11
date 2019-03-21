#pragma once

#include "JWCommon.h"

namespace JWEngine
{
	class JWAssimpLoader final
	{
	public:
		auto LoadObj(STRING Directory, STRING ModelFileName) noexcept->SModelData;
	};
};