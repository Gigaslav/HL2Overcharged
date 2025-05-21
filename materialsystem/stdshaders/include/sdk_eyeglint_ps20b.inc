#ifndef SDK_EYEGLINT_PS20B_H
#define SDK_EYEGLINT_PS20B_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class sdk_eyeglint_ps20b_Static_Index
{
public:
	sdk_eyeglint_ps20b_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_sdk_eyeglint_ps20b 1


class sdk_eyeglint_ps20b_Dynamic_Index
{
public:
	sdk_eyeglint_ps20b_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_sdk_eyeglint_ps20b 1


#endif	// SDK_EYEGLINT_PS20B_H