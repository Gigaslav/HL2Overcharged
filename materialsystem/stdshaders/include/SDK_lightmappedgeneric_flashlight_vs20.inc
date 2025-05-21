// ALL SKIP STATEMENTS THAT AFFECT THIS SHADER!!!
// defined $LIGHTING_PREVIEW && defined $FASTPATH && $LIGHTING_PREVIEW && $FASTPATH

#ifndef SDK_LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H
#define SDK_LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H

#include "shaderapi/ishaderapi.h"
#include "shaderapi/ishadershadow.h"
#include "materialsystem/imaterialvar.h"

class SDK_lightmappedgeneric_flashlight_vs20_Static_Index
{
	unsigned int m_nNORMALMAP : 2;
	unsigned int m_nWORLDVERTEXTRANSITION : 2;
	unsigned int m_nVERTEXCOLOR : 2;
#ifdef _DEBUG
	bool m_bNORMALMAP : 1;
	bool m_bWORLDVERTEXTRANSITION : 1;
	bool m_bVERTEXCOLOR : 1;
#endif	// _DEBUG
public:
	void SetNORMALMAP( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nNORMALMAP = i;
#ifdef _DEBUG
		m_bNORMALMAP = true;
#endif	// _DEBUG
	}

	void SetWORLDVERTEXTRANSITION( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nWORLDVERTEXTRANSITION = i;
#ifdef _DEBUG
		m_bWORLDVERTEXTRANSITION = true;
#endif	// _DEBUG
	}

	void SetVERTEXCOLOR( int i )
	{
		Assert( i >= 0 && i <= 1 );
		m_nVERTEXCOLOR = i;
#ifdef _DEBUG
		m_bVERTEXCOLOR = true;
#endif	// _DEBUG
	}

	SDK_lightmappedgeneric_flashlight_vs20_Static_Index( IShaderShadow* pShaderShadow, IMaterialVar** params )
	{
		m_nNORMALMAP = 0;
		m_nWORLDVERTEXTRANSITION = 0;
		m_nVERTEXCOLOR = 0;
#ifdef _DEBUG
		m_bNORMALMAP = false;
		m_bWORLDVERTEXTRANSITION = false;
		m_bVERTEXCOLOR = false;
#endif	// _DEBUG
	}

	int GetIndex() const
	{
		Assert( m_bNORMALMAP && m_bWORLDVERTEXTRANSITION && m_bVERTEXCOLOR );
		return ( 2 * m_nNORMALMAP ) + ( 4 * m_nWORLDVERTEXTRANSITION ) + ( 8 * m_nVERTEXCOLOR ) + 0;
	}
};

#define shaderStaticTest_SDK_lightmappedgeneric_flashlight_vs20 vsh_forgot_to_set_static_NORMALMAP + vsh_forgot_to_set_static_WORLDVERTEXTRANSITION + vsh_forgot_to_set_static_VERTEXCOLOR


class SDK_lightmappedgeneric_flashlight_vs20_Dynamic_Index
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

	SDK_lightmappedgeneric_flashlight_vs20_Dynamic_Index( IShaderDynamicAPI* pShaderAPI )
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

#define shaderDynamicTest_SDK_lightmappedgeneric_flashlight_vs20 vsh_forgot_to_set_dynamic_DOWATERFOG


#endif	// SDK_LIGHTMAPPEDGENERIC_FLASHLIGHT_VS20_H