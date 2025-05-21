//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: Voltigore and electric sphere clientside effects
//
//=============================================================================//

#include "cbase.h"
#include "model_types.h"
#include "ClientEffectPrecacheSystem.h"
#include "fx.h"
#include "c_te_effect_dispatch.h"
#include "beamdraw.h"

#include "dlight.h"
#include "r_efx.h"

#include "c_ai_basenpc.h"
#include "iefx.h"

//====================================== Voltigore Clientside Effects ============================

class C_NPC_voltigore : public C_AI_BaseNPC
{
public:
	C_NPC_voltigore() {}

	DECLARE_CLASS(C_NPC_voltigore, C_AI_BaseNPC);
	DECLARE_CLIENTCLASS();
	DECLARE_DATADESC();

	virtual void OnDataChanged(DataUpdateType_t type);
	virtual void ClientThink();

private:

	IMaterial *m_pMaterial;
	unsigned char m_iBleedingLevel;
	unsigned char m_iPerformingBleedingLevel;
	CNewParticleEffect *m_pBleedingFX;

	virtual void UpdateBleedingPerformance(void);

	C_NPC_voltigore(const C_NPC_voltigore &);
};


//-----------------------------------------------------------------------------
// Save/restore
//-----------------------------------------------------------------------------
BEGIN_DATADESC(C_NPC_voltigore)
END_DATADESC()


//-----------------------------------------------------------------------------
// Networking
//-----------------------------------------------------------------------------
IMPLEMENT_CLIENTCLASS_DT(C_NPC_voltigore, DT_NPC_voltigore, CNPC_voltigore)

RecvPropInt(RECVINFO(m_iBleedingLevel)),
END_RECV_TABLE()


//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_voltigore::OnDataChanged(DataUpdateType_t type)
{
	BaseClass::OnDataChanged(type);

	if (m_iBleedingLevel != m_iPerformingBleedingLevel)
	{
		UpdateBleedingPerformance();
	}

}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_voltigore::UpdateBleedingPerformance()
{
	CParticleProperty * pProp = ParticleProp();

	if (m_pBleedingFX)
	{
		pProp->StopEmission(m_pBleedingFX);
		m_pBleedingFX = NULL;
	}

	switch (m_iBleedingLevel)
	{
	case 1: // light bleeding
	{
		m_pBleedingFX = pProp->Create("blood_antlionguard_injured_light", PATTACH_ABSORIGIN_FOLLOW);
		AssertMsg1(m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_light");
		if (m_pBleedingFX)
		{
			pProp->AddControlPoint(m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW);
		}
	}
	break;

	case 2: // severe bleeding
	{
		m_pBleedingFX = pProp->Create("blood_antlionguard_injured_heavy", PATTACH_ABSORIGIN_FOLLOW);
		AssertMsg1(m_pBleedingFX, "Particle system couldn't make %s", "blood_antlionguard_injured_heavy");
		if (m_pBleedingFX)
		{
			pProp->AddControlPoint(m_pBleedingFX, 1, this, PATTACH_ABSORIGIN_FOLLOW);
		}

	}
	break;
	}

	m_iPerformingBleedingLevel = m_iBleedingLevel;
}

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
void C_NPC_voltigore::ClientThink()
{
	BaseClass::ClientThink();
}

//====================================== Electric Sphere ============================

class C_VoltElectricSphere : public C_BaseCombatCharacter
{
	DECLARE_CLASS( C_VoltElectricSphere, C_BaseCombatCharacter );
	DECLARE_CLIENTCLASS();
public:
	
	C_VoltElectricSphere( void );

	virtual RenderGroup_t GetRenderGroup( void )
	{
		return RENDER_GROUP_TWOPASS;
	}

	virtual void	ClientThink( void );

	virtual void	OnDataChanged( DataUpdateType_t updateType );
	virtual int		DrawModel( int flags );

private:

	C_VoltElectricSphere( const C_VoltElectricSphere & );

	Vector	m_vecLastOrigin;
	bool	m_bUpdated;
};

IMPLEMENT_CLIENTCLASS_DT( C_VoltElectricSphere, DT_VoltElectricSphere, CVoltElectricSphere )
END_RECV_TABLE()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_VoltElectricSphere::C_VoltElectricSphere( void )
{
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : updateType - 
//-----------------------------------------------------------------------------
void C_VoltElectricSphere::OnDataChanged( DataUpdateType_t updateType )
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_bUpdated = false;
		m_vecLastOrigin = GetAbsOrigin();
		SetNextClientThink( CLIENT_THINK_ALWAYS );
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : flags - 
// Output : int
//-----------------------------------------------------------------------------
int C_VoltElectricSphere::DrawModel( int flags )
{
	if ( flags & STUDIO_TRANSPARENCY )
	{
		Vector	vecDir = GetAbsOrigin() - m_vecLastOrigin;
		float	speed = VectorNormalize( vecDir );
		
		speed = clamp( speed, 0, 32 );
		
		if ( speed > 0 )
		{
			float	stepSize = min( ( speed * 0.5f ), 4.0f );

			Vector	spawnPos = GetAbsOrigin() + ( vecDir * 24.0f );
			Vector	spawnStep = -vecDir * stepSize;

			{
				dlight_t *dl = effects->CL_AllocDlight(index);	// BriJee : dlight
				dl->origin = spawnPos;	//effect_origin;
				dl->color.r = 148;
				dl->color.g = 0;
				dl->color.b = 211;
				dl->color.exponent = 5;
				dl->radius = 150.0f * (engine->MapHasHDRLighting() ? 1 : 0.3);
				dl->die = gpGlobals->curtime + 0.1f; //0.2f;
				dl->decay = 512.0f;//0.05f; //0.0f;
			}
		}

		if ( gpGlobals->frametime > 0.0f && !m_bUpdated)
		{
			m_bUpdated = true;
			m_vecLastOrigin = GetAbsOrigin();
		}

		return 1;
	}

	return BaseClass::DrawModel( flags );
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_VoltElectricSphere::ClientThink( void )
{
	m_bUpdated = false;
}
