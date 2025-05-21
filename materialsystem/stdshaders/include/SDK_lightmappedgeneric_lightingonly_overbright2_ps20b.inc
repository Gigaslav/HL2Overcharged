#ifndef SDK_LIGHTMAPPEDGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H
#define SDK_LIGHTMAPPEDGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class SDK_lightmappedgeneric_lightingonly_overbright2_ps20b_Static_Index
{
public:
	SDK_lightmappedgeneric_lightingonly_overbright2_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_SDK_lightmappedgeneric_lightingonly_overbright2_ps20b 1


class SDK_lightmappedgeneric_lightingonly_overbright2_ps20b_Dynamic_Index
{
public:
	SDK_lightmappedgeneric_lightingonly_overbright2_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_SDK_lightmappedgeneric_lightingonly_overbright2_ps20b 1


#endif	// SDK_LIGHTMAPPEDGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H