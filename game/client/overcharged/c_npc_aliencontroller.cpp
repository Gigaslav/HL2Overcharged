//========= Copyright © 1996-2005, Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=====================================================================================//

#include "cbase.h"
#include "particles_simple.h"
#include "citadel_effects_shared.h"
#include "particles_attractor.h"
#include "iefx.h"
#include "dlight.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_te_effect_dispatch.h"
#include "fx_quad.h"

#include "c_ai_basenpc.h"
 
// For material proxy
#include "ProxyEntity.h"
#include "materialsystem/IMaterial.h"
#include "materialsystem/IMaterialVar.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

ConVar aliencontroller_fireball_dlight("oc_aliencontroller_fireball_dlight", "1");

#define ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS 3

#define NUM_INTERIOR_PARTICLES	8

#define DLIGHT_RADIUS (150.0f)
#define DLIGHT_MINLIGHT (40.0f/255.0f)
#define HLSS_CONTROLLER_TELEKINESIS 1 // Light Kill : Enable telekinesis for Alien Controller


class C_NPC_AlienController : public C_AI_BaseNPC
{
	DECLARE_CLASS( C_NPC_AlienController, C_AI_BaseNPC );
	DECLARE_CLIENTCLASS();

public:
	C_NPC_AlienController();

	virtual void	ClientThink( void );
	virtual void	UpdateOnRemove( void );
	virtual void	OnRestore();
	//virtual void	ReceiveMessage( int classID, bf_read &msg );
	virtual void	OnDataChanged( DataUpdateType_t updateType );

private:
	
	CNewParticleEffect				*m_hRightHandEffect;
	CNewParticleEffect				*m_hLeftHandEffect;

#ifdef HLSS_CONTROLLER_TELEKINESIS
	CNewParticleEffect	*m_hPhysicsEffect[ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS];
	EHANDLE				m_hPhysicsEnt[ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS];
#endif

	dlight_t *m_pELight;

	int	m_nRightHandAttachment;
	int m_nLeftHandAttachment;
	int m_nGlowAttachment;

	bool m_bFireballEffects;
};

IMPLEMENT_CLIENTCLASS_DT( C_NPC_AlienController, DT_NPC_AlienController, CNPC_AlienController )
	RecvPropBool( RECVINFO( m_bFireballEffects ) ),
#ifdef HLSS_CONTROLLER_TELEKINESIS
	RecvPropArray3
	(
		RECVINFO_ARRAY( m_hPhysicsEnt ),
		RecvPropEHandle (RECVINFO(m_hPhysicsEnt[0]))
	),
#endif
END_RECV_TABLE()

C_NPC_AlienController::C_NPC_AlienController()
{
	m_bFireballEffects = false;
	m_pELight = NULL;

#ifdef HLSS_CONTROLLER_TELEKINESIS
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		m_hPhysicsEffect[i] = NULL;
		m_hPhysicsEnt[i] = NULL;
	}
#endif
}

//------------------------------------------------------------------------------
// Purpose :
// Input   :
// Output  :
//------------------------------------------------------------------------------
void C_NPC_AlienController::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged( updateType );

	if ( updateType == DATA_UPDATE_CREATED )
	{
		m_nRightHandAttachment = LookupAttachment("RightHand");
		m_nLeftHandAttachment = LookupAttachment("LeftHand");
		m_nGlowAttachment = LookupAttachment("FireBallGlow");
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

void C_NPC_AlienController::OnRestore()
{
	BaseClass::OnRestore();

	m_nRightHandAttachment = LookupAttachment("RightHand");
	m_nLeftHandAttachment = LookupAttachment("LeftHand");
	m_nGlowAttachment = LookupAttachment("FireBallGlow");

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_NPC_AlienController::ClientThink( void )
{
	// Don't update if our frame hasn't moved forward (paused)
	if ( gpGlobals->frametime <= 0.0f ) //0.0f
		return;

#ifdef HLSS_CONTROLLER_TELEKINESIS
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		if (m_hPhysicsEnt[i]) // && m_hPhysicsEnt[i]->VPhysicsGetObject() && !m_hPhysicsEnt[i]->VPhysicsGetObject()->IsAsleep())
		{
			//TERO: testing this stuff here, remove later
			// Place a beam between the two points //m_pEnt->
			if (!m_hPhysicsEffect[i])
			{
				CNewParticleEffect *pEffect = ParticleProp()->Create( "controller_telekinesis", PATTACH_ABSORIGIN );
				m_hPhysicsEffect[i] = pEffect;
			}

			if (m_hPhysicsEffect[i])
			{
				Vector mins, maxs;
				m_hPhysicsEnt[i]->GetRenderBounds(mins, maxs);

				Vector vecOrigin = m_hPhysicsEnt[i]->GetAbsOrigin();
				Vector vecStart = vecOrigin + Vector(0,0,maxs.z);

				m_hPhysicsEffect[i]->SetControlPoint( 0, vecStart );

				//debugoverlay->AddBoxOverlay( vecStart, -Vector(10,10,10), Vector(10,10,10), vec3_angle, 255,0,0, 0, 0.1 );

				Vector vecPoint = vecStart + Vector( mins.x, mins.y, -8 );
				m_hPhysicsEffect[i]->SetControlPoint( 1, vecPoint );

				//debugoverlay->AddBoxOverlay( vecPoint, -Vector(10,10,10), Vector(10,10,10), vec3_angle, 255,0,0, 0, 0.1 );

				vecPoint = vecStart + Vector( mins.x, maxs.y, -8 );
				m_hPhysicsEffect[i]->SetControlPoint( 2, vecPoint );

				//debugoverlay->AddBoxOverlay( vecPoint, -Vector(10,10,10), Vector(10,10,10), vec3_angle, 255,0,0, 0, 0.1 );

				vecPoint = vecStart + Vector( maxs.x, maxs.y, -8 );
				m_hPhysicsEffect[i]->SetControlPoint( 3, vecPoint );

				//debugoverlay->AddBoxOverlay( vecPoint, -Vector(10,10,10), Vector(10,10,10), vec3_angle, 255,0,0, 0, 0.1 );
				
				vecPoint = vecStart + Vector( maxs.x, mins.y, -8 );
				m_hPhysicsEffect[i]->SetControlPoint( 4, vecPoint );

				//debugoverlay->AddBoxOverlay( vecPoint, -Vector(10,10,10), Vector(10,10,10), vec3_angle, 255,0,0, 0, 0.1 );
			}
		}
		else if (m_hPhysicsEffect[i])
		{
			m_hPhysicsEffect[i]->StopEmission();
			m_hPhysicsEffect[i] = NULL;
		}
	}
#endif

	if (m_bFireballEffects)
	{
		if (!m_hRightHandEffect)
		{		
			// Get our attachment position
			Vector vecStart;
			QAngle vecAngles;
			GetAttachment( m_nRightHandAttachment, vecStart, vecAngles );

			// Place a beam between the two points //m_pEnt->
			CNewParticleEffect *pEffect = ParticleProp()->Create( "controller_fireball", PATTACH_POINT_FOLLOW, m_nRightHandAttachment );
			if ( pEffect )
			{
				pEffect->SetControlPoint( 0, vecStart );

				m_hRightHandEffect = pEffect;
			}
		}

		if (!m_hLeftHandEffect)
		{		
			// Get our attachment position
			Vector vecStart;
			QAngle vecAngles;
			GetAttachment( m_nLeftHandAttachment, vecStart, vecAngles );

			// Place a beam between the two points //m_pEnt->
			CNewParticleEffect *pEffect = ParticleProp()->Create( "controller_fireball", PATTACH_POINT_FOLLOW, m_nLeftHandAttachment );
			if ( pEffect )
			{
				pEffect->SetControlPoint( 0, vecStart );

				m_hLeftHandEffect = pEffect;
			}
		}

	
		if (aliencontroller_fireball_dlight.GetBool())
		{
			if (!m_pELight)
			{
				m_pELight = effects->CL_AllocElight( entindex() );
				m_pELight->die		= FLT_MAX;
			}

			if (m_pELight)
			{
				Vector effect_origin;
				GetAttachment(m_nGlowAttachment, effect_origin);

				//DevMsg("ELight created, attachment index %d\n", m_nGlowAttachment);

				m_pELight->origin	= effect_origin;
				m_pELight->radius	= 100.0f; //DLIGHT_RADIUS;
				m_pELight->minlight = (10/255.0f);
				m_pELight->color.r = 255;
				m_pELight->color.g = 128;
				m_pELight->color.b = 0;
			}
		}
	}
	else
	{
		if ( m_hLeftHandEffect )
		{
			m_hLeftHandEffect->StopEmission();
			m_hLeftHandEffect = NULL;
		}

		if ( m_hRightHandEffect )
		{
			m_hRightHandEffect->StopEmission();
			m_hRightHandEffect = NULL;
		}

		if ( m_pELight )
		{
			m_pELight->die = gpGlobals->curtime;
			m_pELight = NULL;
		}

		SetNextClientThink( CLIENT_THINK_NEVER );
	}
}


void C_NPC_AlienController::UpdateOnRemove( void )
{
	if ( m_hLeftHandEffect )
	{
		m_hLeftHandEffect->StopEmission();
		m_hLeftHandEffect = NULL;
	}

	if ( m_hRightHandEffect )
	{
		m_hRightHandEffect->StopEmission();
		m_hRightHandEffect = NULL;
	}

	if ( m_pELight )
	{
		m_pELight->die = gpGlobals->curtime;
		m_pELight = NULL;
	}

#ifdef HLSS_CONTROLLER_TELEKINESIS
	for (int i=0; i<ALIENCONTROLLER_NUMBER_OF_TELEKINESIS_OBJECTS; i++)
	{
		if (m_hPhysicsEffect[i])
		{
			m_hPhysicsEffect[i]->StopEmission();
			m_hPhysicsEffect[i] = NULL;
		}
	}
#endif

	BaseClass::UpdateOnRemove();
}
