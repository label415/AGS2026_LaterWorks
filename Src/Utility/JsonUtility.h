#pragma once
#include <DxLib.h>
#include "../Manager/Resource.h"

namespace DxLib
{
	// DxLib‚ÌVECTOR(tagVECTOR)‚Ænlohmann::json‚ÌŽ©“®•ÏŠ·
	inline void to_json(Json& j, const tagVECTOR& v)
	{
		j = Json{ { "x", v.x }, { "y", v.y }, { "z", v.z } };
	}

	inline void from_json(const Json& j, tagVECTOR& v)
	{
		v.x = j.value("x", 0.0f);
		v.y = j.value("y", 0.0f);
		v.z = j.value("z", 0.0f);
	}
}
