// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_SHATTEREDGLASS_VS20_H
#define SDK_SHATTEREDGLASS_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class SDK_ShatteredGlass_vs20_Static_Index
{
	unsigned int m_nENVMAP_MASK : 2;
#ifdef _DEBUG
	bool m_bENVMAP_MASK : 1;
#endif	// _DEBUG
public:
	void SetENVMAP_MASK( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nENVMAP_MASK = i;
#ifdef _DEBUG
		m_bENVMAP_MASK = true;
#endif	// _DEBUG
	}

	SDK_ShatteredGlass_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nENVMAP_MASK = 0;
#ifdef _DEBUG
		m_bENVMAP_MASK = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bENVMAP_MASK );
		return ( 2 * m_nENVMAP_MASK ) + 0;
	}
};

#define shaderStaticTest_SDK_ShatteredGlass_vs20 vsh_forgot_to_set_static_ENVMAP_MASK


class SDK_ShatteredGlass_vs20_Dynamic_Index
{
	unsigned int m_nDOWATERFOG : 2;
#ifdef _DEBUG
	bool m_bDOWATERFOG : 1;
#endif	// _DEBUG
public:
	void SetDOWATERFOG( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nDOWATERFOG = i;
#ifdef _DEBUG
		m_bDOWATERFOG = true;
#endif	// _DEBUG
	}

	SDK_ShatteredGlass_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
	{
		m_nDOWATERFOG = 0;
#ifdef _DEBUG
		m_bDOWATERFOG = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bDOWATERFOG );
		return ( 1 * m_nDOWATERFOG ) + 0;
	}
};

#define shaderDynamicTest_SDK_ShatteredGlass_vs20 vsh_forgot_to_set_dynamic_DOWATERFOG


#endif	// SDK_SHATTEREDGLASS_VS20_H