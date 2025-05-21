// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef DEFERRED_LIGHTNING_VS20_H
#define DEFERRED_LIGHTNING_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class deferred_lightning_vs20_Static_Index
{
public:
	deferred_lightning_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderStaticTest_deferred_lightning_vs20 1


class deferred_lightning_vs20_Dynamic_Index
{
public:
	deferred_lightning_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
	}

	int GetIndex() const
	{
		return 0;
	}
};

#define shaderDynamicTest_deferred_lightning_vs20 1


#endif	// DEFERRED_LIGHTNING_VS20_H