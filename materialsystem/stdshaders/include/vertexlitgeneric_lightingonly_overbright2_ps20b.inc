#ifndef VERTEXLITGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H
#define VERTEXLITGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class vertexlitgeneric_lightingonly_overbright2_ps20b_Static_Index
{
public:
	vertexlitgeneric_lightingonly_overbright2_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_vertexlitgeneric_lightingonly_overbright2_ps20b 1


class vertexlitgeneric_lightingonly_overbright2_ps20b_Dynamic_Index
{
public:
	vertexlitgeneric_lightingonly_overbright2_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_vertexlitgeneric_lightingonly_overbright2_ps20b 1


#endif	// VERTEXLITGENERIC_LIGHTINGONLY_OVERBRIGHT2_PS20B_H