// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEBUGMORPHACCUMULATOR_VS20_H
#define DEBUGMORPHACCUMULATOR_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class debugmorphaccumulator_vs20_Static_Index
{
public:
	debugmorphaccumulator_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_debugmorphaccumulator_vs20 1


class debugmorphaccumulator_vs20_Dynamic_Index
{
public:
	debugmorphaccumulator_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_debugmorphaccumulator_vs20 1


#endif	// DEBUGMORPHACCUMULATOR_VS20_H