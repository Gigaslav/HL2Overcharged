//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_ar2_muzzleflash_effect.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"

#include "c_basehlplayer.h"

//#include "tempent.h"
#ifdef HL2_CLIENT_DLL
#include "c_basehlplayer.h"
#include "convar.h"
#endif // HL2_CLIENT_DLL

#if defined( _X360 )
extern ConVar r_flashlightdepthres;
#else
extern ConVar r_flashlightdepthres;
#endif

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

extern ConVar r_flashlightdepthtexture;

void r_Ar2_newMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
//ConVar   weapon_muzzleflash_effect("weapon_muzzleflash_effect", "0");
static ConVar r_Ar2_newM("r_Ar2_newM", "1", FCVAR_CHEAT, "", r_Ar2_newMCallback_f);
static ConVar r_Ar2_swingM("r_Ar2_swingM", "1", FCVAR_CHEAT);
static ConVar r_Ar2_Mlockposition("r_Ar2_Mlockposition", "0", FCVAR_CHEAT);
static ConVar r_Ar2_MDynamicFov("r_Ar2_MDynamicFov", "1", FCVAR_CHEAT);
static ConVar r_Ar2_Mfov("r_Ar2_Mfov", "130.0", FCVAR_CHEAT);
static ConVar r_Ar2_MfovTimer("r_Ar2_MfovTimer", "10.0", FCVAR_CHEAT);
static ConVar r_Ar2_MfovTimerBorder("r_Ar2_MfovTimerBorder", "80.0", FCVAR_CHEAT);
static ConVar r_Ar2_Moffsetx("r_Ar2_Moffsetx", "0.0", FCVAR_CHEAT);
static ConVar r_Ar2_Moffsety("r_Ar2_Moffsety", "0.55", FCVAR_CHEAT);
static ConVar r_Ar2_Moffsetz("r_Ar2_Moffsetz", "-17.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mnear("r_Ar2_Mnear", "4.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mfar("r_Ar2_Mfar", "350.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mconstant("r_Ar2_Mconstant", "0.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mlinear("r_Ar2_Mlinear", "100", FCVAR_CHEAT);
static ConVar r_Ar2_MDynamiclinear("r_Ar2_MDynamiclinear", "0", FCVAR_CHEAT);
static ConVar r_Ar2_Mquadratic("r_Ar2_Mquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mvisualizetrace("r_Ar2_Mvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_Ar2_Mambient("r_Ar2_Mambient", "0.0", FCVAR_CHEAT);
static ConVar r_Ar2_Mshadowatten("r_Ar2_Mshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_Mladderdist("r_Mladderdist", "40.0", FCVAR_REPLICATED | FCVAR_CHEAT);
static ConVar mat_Ar2_slopescaledepthbias_shadowmap_M("mat_Ar2_slopescaledepthbias_shadowmap_M", "16", FCVAR_CHEAT);
static ConVar mat_Ar2_depthbias_shadowmap_M("mat_Ar2_depthbias_shadowmap_M", "0.0005", FCVAR_CHEAT);


void r_Ar2_newMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_Ar2_newM.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_Ar2_MuzzleflashEffect::C_Ar2_MuzzleflashEffect(int AR2_nEntIndex)
{
	m_Ar2_MHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_Ar2_nEntIndexM = AR2_nEntIndex;


	m_Ar2_M = false;

	int j = random->RandomInt(1, 4);


	m_Ar2_pPointLightM = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_Ar2_newM.SetValue(0);
	}


	if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
	{
		m_Ar2_MTexture.Init("effects/flashlight_border", TEXTURE_GROUP_OTHER, true);
	}
	else
	{


		//    if ( i == 1 )
		//    {

		if (j == 1)
		{
			m_Ar2_MTexture.Init("effects/ar2muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
		}
		if (j == 2)
		{
			m_Ar2_MTexture.Init("effects/ar2muzzleflash_light2", TEXTURE_GROUP_OTHER, true);
		}
		if (j == 3)
		{
			m_Ar2_MTexture.Init("effects/ar2muzzleflash_light3", TEXTURE_GROUP_OTHER, true);
		}
		if (j == 4)
		{
			m_Ar2_MTexture.Init("effects/ar2muzzleflash_light4", TEXTURE_GROUP_OTHER, true);
		}


		//   }


		/*
		if ( i == 2 )
		{

		if ( j == 1 )
		{
		m_MTexture.Init( "effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true );
		}
		if ( j == 2 )
		{
		m_MTexture.Init( "effects/muzzleflash_light2", TEXTURE_GROUP_OTHER, true );
		}
		if ( j == 3 )
		{
		m_MTexture.Init( "effects/muzzleflash_light3", TEXTURE_GROUP_OTHER, true );
		}
		if ( j == 4 )
		{
		m_MTexture.Init( "effects/muzzleflash_light4", TEXTURE_GROUP_OTHER, true );
		}

		}
		*/


	}
	//	if (i = 1)
	//	{
	/*
	if ( g_pMaterialSystemHardwareConfig->SupportsBorderColor() )
	{
	m_MTexture.Init( "effects/flashlight_border", TEXTURE_GROUP_OTHER, true );
	}
	else
	{

	int j = random->RandomInt( 1, 4 );
	if ( j == 1 )
	{
	m_MTexture.Init( "effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true );
	}
	if ( j == 2 )
	{
	m_MTexture.Init( "effects/muzzleflash_light2", TEXTURE_GROUP_OTHER, true );
	}
	if ( j == 3 )
	{
	m_MTexture.Init( "effects/muzzleflash_light3", TEXTURE_GROUP_OTHER, true );
	}
	if ( j == 4 )
	{
	m_MTexture.Init( "effects/muzzleflash_light4", TEXTURE_GROUP_OTHER, true );
	}

	}


	}




	//	if (i = 2)
	//	{


	if ( g_pMaterialSystemHardwareConfig->SupportsBorderColor() )
	{
	m_MTexture.Init( "effects/flashlight_border", TEXTURE_GROUP_OTHER, true );
	}
	else
	{

	//   int j = random->RandomInt( 1, 4 );
	//   if ( j == 1 )
	//   {
	m_MTexture.Init( "effects/flashlight001", TEXTURE_GROUP_OTHER, true );
	/*   }
	if ( j == 2 )
	{
	m_MTexture.Init( "effects/muzzleflash_light2", TEXTURE_GROUP_OTHER, true );
	}
	if ( j == 3 )
	{
	m_MTexture.Init( "effects/muzzleflash_light3", TEXTURE_GROUP_OTHER, true );
	}
	if ( j == 4 )
	{
	m_MTexture.Init( "effects/muzzleflash_light4", TEXTURE_GROUP_OTHER, true );
	}

	}
	*/

	//	}
	//	}

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Ar2_MuzzleflashEffect::~C_Ar2_MuzzleflashEffect()
{
	Ar2_LightOffM();

	/*		if( engine->GetDXSupportLevel() < 70 )
	{
	r_newflashlight.SetValue( 0 );
	}
	if  ( weapon_laser_pointer.GetFloat() == true )
	{
	m_LaserTexture.Init( "sprites/redglow1", TEXTURE_GROUP_OTHER, true );

	}*/
}

























//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_TurnOnM()
{
	m_Ar2_M = true;
	m_Ar2_flDistModM = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_TurnOffM()
{
	if (m_Ar2_M)
	{
		m_Ar2_M = false;
		Ar2_LightOffM();
	}

	/*		if ( IsActive() == false )
	{
	m_M = false;
	LightOffM();
	}*/
}




// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
class CTraceFilterSkipPlayerAndViewModelAR2 : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(IHandleEntity *pServerEntity, int contentsMask)
	{
		// Test against the vehicle too?
		// FLASHLIGHTFIXME: how do you know that you are actually inside of the vehicle?
		C_BaseEntity *pEntity = EntityFromEntityHandle(pServerEntity);
		if (!pEntity)
			return true;

		if ((dynamic_cast<C_BaseViewModel *>(pEntity) != NULL) ||
			(dynamic_cast<C_BasePlayer *>(pEntity) != NULL) ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_DEBRIS ||
			pEntity->GetCollisionGroup() == COLLISION_GROUP_INTERACTIVE_DEBRIS)
		{
			return false;
		}

		return true;
	}
};
//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_UpdateLightNewM(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	VPROF_BUDGET("C_Ar2_MuzzleflashEffect::Ar2_UpdateLightNewM", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float Ar2_flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float Ar2_flDistCutoff = 128.0f;
	const float Ar2_flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelAR2 traceFilter;
	float Ar2_flOffsetY = r_Ar2_Moffsety.GetFloat();
	float Ar2_flOffsetZ = r_Ar2_Moffsetz.GetFloat();
	float Ar2_flOffsetX = r_Ar2_Moffsetx.GetFloat();

	//    m_flMuzzleFlashRoll = RandomFloat( 0, 360.0f );

	if (r_Ar2_swingM.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			//		flOffsetY += (vecSwingLight.z - vecPos.z);//Вот оно////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}

	/*	if( r_swingM.GetBool() )
	{
	// This projects the view direction backwards, attempting to raise the vertical
	// offset of the flashlight, but only when the player is looking down.
	Vector vecSwingLight = vecPos + vecForward * -12.0f;
	if( vecSwingLight.z > vecPos.z )
	{
	flOffsetY += (vecSwingLight.z - vecPos.z);
	}
	}
	*//*
	m_flMuzzleFlashRoll = RandomFloat( -360.0f, 360.0f );

	C_BasePlayer *player = C_BasePlayer::GetLocalPlayer();
	if ( !player )
	return;

	Vector vecForward1, vecRight1, vecUp1, vecPos1;
	vecPos1 = player->EyePosition();
	player->EyeVectors( &vecForward1, &vecRight1, &vecUp1 );


	QAngle ang;
	VectorAngles( vecForward1, vecUp1, ang );
	ang.z = m_flMuzzleFlashRoll;
	AngleVectors( ang, &vecForward1, &vecRight1, &vecUp1 );
	*/
	//	Vector vOrigin = vecPos + flOffsetY * vecUp;
	//	Vector vOrigin = vecPos1 + ((flOffsetZ * vecForward1) + flOffsetX * vecUp1 + (flOffsetY * vecRight1));
	/////////////////////	Vector vOrigin = vecPos + Ar2_flOffsetY * vecUp;
	Vector vOrigin = vecPos + ((Ar2_flOffsetZ * vecForward) + Ar2_flOffsetX * vecUp + (Ar2_flOffsetY * vecRight));
	// Not on ladder...trace a hull
	if (!bPlayerOnLadder)
	{
		trace_t pmOriginTrace;
		UTIL_TraceHull(vecPos, vOrigin, Vector(-4, -4, -4), Vector(4, 4, 4), MASK_SOLID & ~(CONTENTS_HITBOX), &traceFilter, &pmOriginTrace);

		if (pmOriginTrace.DidHit())
		{
			vOrigin = vecPos;
		}
	}
	else // on ladder...skip the above hull trace
	{
		vOrigin = vecPos;
	}

	// Now do a trace along the flashlight direction to ensure there is nothing within range to pull back from
	int iMask = MASK_OPAQUE_AND_NPCS;
	iMask &= ~CONTENTS_HITBOX;
	iMask |= CONTENTS_WINDOW;

	Vector vTarget = vecPos + vecForward * r_Ar2_Mfar.GetFloat();

	// Work with these local copies of the basis for the rest of the function
	Vector vDir = vTarget - vOrigin;
	Vector vRight = vecRight;
	Vector vUp = vecUp;
	VectorNormalize(vDir);
	VectorNormalize(vRight);
	VectorNormalize(vUp);

	// Orthonormalize the basis, since the flashlight texture projection will require this later...
	vUp -= DotProduct(vDir, vUp) * vDir;
	VectorNormalize(vUp);
	vRight -= DotProduct(vDir, vRight) * vDir;
	VectorNormalize(vRight);
	vRight -= DotProduct(vUp, vRight) * vUp;
	VectorNormalize(vRight);

	AssertFloatEquals(DotProduct(vDir, vRight), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vDir, vUp), 0.0f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);

	trace_t pmDirectionTrace;
	UTIL_TraceHull(vOrigin, vTarget, Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmDirectionTrace);

	if (r_Ar2_Mvisualizetrace.GetBool() == true)
	{
		debugoverlay->AddBoxOverlay(pmDirectionTrace.endpos, Vector(-4, -4, -4), Vector(4, 4, 4), QAngle(0, 0, 0), 0, 0, 255, 16, 0);
		debugoverlay->AddLineOverlay(vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0);
	}

	float Ar2_flDist = (pmDirectionTrace.endpos - vOrigin).Length();
	if (Ar2_flDist < Ar2_flDistCutoff)
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_Mladderdist.GetFloat() : Ar2_flDistCutoff - Ar2_flDist;	// Fixed pull-back distance if on ladder
		m_Ar2_flDistModM = Lerp(Ar2_flDistDrag, m_Ar2_flDistModM, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - Ar2_flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - Ar2_flEpsilon;
				if (m_Ar2_flDistModM > flMaxDist)
					m_Ar2_flDistModM = flMaxDist;
			}
		}
	}
	else
	{
		m_Ar2_flDistModM = Lerp(Ar2_flDistDrag, m_Ar2_flDistModM, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_Ar2_flDistModM;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_Ar2_Mquadratic.GetFloat();

	bool bFlicker = false;

	if (r_Ar2_MDynamicFov.GetInt())
	{
		float Timeing = RandomFloat(-10.0f, 10.0f);
		mFov = mFov + r_Ar2_MfovTimer.GetFloat();
		if (mFov >= (r_Ar2_MfovTimerBorder.GetFloat() + Timeing))
			mFov = 0.0f;

		if (bFlicker == false)
		{
			if (r_Ar2_MDynamiclinear.GetInt())
			{
				int Rl1 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (r_Ar2_Mlinear.GetFloat() * Rl1);
			}
			if (!(r_Ar2_MDynamiclinear.GetInt()))
				state.m_fLinearAtten = r_Ar2_Mlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_Ar2_Mfov.GetFloat() - mFov);
			state.m_fVerticalFOVDegrees = (r_Ar2_Mfov.GetFloat() - mFov);
		}
	}
	if (!(r_Ar2_MDynamicFov.GetInt()))
	{
		int R = RandomFloat(-8, 8);
		if (bFlicker == false)
		{
			if (r_Ar2_MDynamiclinear.GetInt())
			{
				int Rl2 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (r_Ar2_Mlinear.GetFloat() * Rl2);
			}
			if (!(r_Ar2_MDynamiclinear.GetInt()))
				state.m_fLinearAtten = r_Ar2_Mlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_Ar2_Mfov.GetFloat() - R);
			state.m_fVerticalFOVDegrees = (r_Ar2_Mfov.GetFloat() - R);
		}
	}

	state.m_fConstantAtten = r_Ar2_Mconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_Ar2_Mambient.GetFloat();
	state.m_NearZ = r_Ar2_Mnear.GetFloat() + m_Ar2_flDistModM;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_Ar2_Mfar.GetFloat();
	state.m_bEnableShadows = cvar->FindVar("r_flashlightShadows")->GetBool();//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_Ar2_MTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;
	//	state.m_nSpotLaserTextureFrame = 0;


	state.m_flShadowAtten = r_Ar2_Mshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_Ar2_slopescaledepthbias_shadowmap_M.GetFloat();
	state.m_flShadowDepthBias = mat_Ar2_depthbias_shadowmap_M.GetFloat();

	if (m_Ar2_MHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_Ar2_MHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_Ar2_Mlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_Ar2_MHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_Ar2_MHandle, true);

	// Kill the old flashlight method if we have one.
	Ar2_LightOffOldM();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("Ar2_MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Ar2_entindex", m_Ar2_nEntIndexM);
		msg->SetInt("Ar2_MHandle", m_Ar2_MHandle);
		msg->SetPtr("Ar2_MState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_Ar2_pPointLightM || (m_Ar2_pPointLightM->key != m_Ar2_nEntIndexM))
	{
		// Set up the environment light
		m_Ar2_pPointLightM = effects->CL_AllocDlight(m_Ar2_nEntIndexM);
		m_Ar2_pPointLightM->flags = 0.0f;
		m_Ar2_pPointLightM->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_Ar2_pPointLightM->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelAR2 traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_Ar2_pPointLightM->origin);

	float Ar2_falloff = pm.fraction * nDistance;

	if (Ar2_falloff < 500)
		Ar2_falloff = 1.0;
	else
		Ar2_falloff = 500.0 / Ar2_falloff;

	Ar2_falloff *= Ar2_falloff;

	m_Ar2_pPointLightM->radius = 80;
	m_Ar2_pPointLightM->color.r = m_Ar2_pPointLightM->color.g = m_Ar2_pPointLightM->color.b = 255 * Ar2_falloff;
	m_Ar2_pPointLightM->color.exponent = 0;

	// Make it live for a bit
	m_Ar2_pPointLightM->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_Ar2_pPointLightM);

	// kill the new flashlight if we have one
	Ar2_LightOffNewM();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
{
	if (!m_Ar2_M)
	{
		return;
	}
	if (r_Ar2_newM.GetBool())
	{
		Ar2_UpdateLightNewM(vecPos, vecDir, vecRight, vecUp);
	}
	else
	{
		Ar2_UpdateLightOldM(vecPos, vecDir, nDistance);
	}

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_LightOffNewM()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("Ar2_MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Ar2_entindex", m_Ar2_nEntIndexM);
		msg->SetInt("Ar2_MHandle", m_Ar2_MHandle);
		msg->SetPtr("Ar2_MState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_Ar2_MHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_Ar2_MHandle);
		m_Ar2_MHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_LightOffOldM()
{
	if (m_Ar2_pPointLightM && (m_Ar2_pPointLightM->key == m_Ar2_nEntIndexM))
	{
		m_Ar2_pPointLightM->die = gpGlobals->curtime;
		m_Ar2_pPointLightM = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_MuzzleflashEffect::Ar2_LightOffM()
{
	Ar2_LightOffOldM();
	Ar2_LightOffNewM();
}
/*
CHeadlightEffect::CHeadlightEffect()
{

}

CHeadlightEffect::~CHeadlightEffect()
{

}

void CHeadlightEffect::UpdateLight( const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance )
{
if ( IsOn() == false )
return;

FlashlightState_t state;
Vector basisX, basisY, basisZ;
basisX = vecDir;
basisY = vecRight;
basisZ = vecUp;
VectorNormalize(basisX);
VectorNormalize(basisY);
VectorNormalize(basisZ);

BasisToQuaternion( basisX, basisY, basisZ, state.m_quatOrientation );

state.m_vecLightOrigin = vecPos;

state.m_fHorizontalFOVDegrees = 45.0f;
state.m_fVerticalFOVDegrees = 30.0f;
state.m_fQuadraticAtten = r_flashlightquadratic.GetFloat();
state.m_fLinearAtten = r_flashlightlinear.GetFloat();
state.m_fConstantAtten = r_flashlightconstant.GetFloat();
state.m_Color[0] = 1.0f;
state.m_Color[1] = 1.0f;
state.m_Color[2] = 1.0f;
state.m_Color[3] = r_flashlightambient.GetFloat();
state.m_NearZ = r_flashlightnear.GetFloat();
state.m_FarZ = r_flashlightfar.GetFloat();
state.m_bEnableShadows = true;
state.m_pSpotlightTexture = m_FlashlightTexture;
//	state.m_pLaserTexture = m_LaserTexture;
state.m_nSpotlightTextureFrame = 0;
//	state.m_nSpotLaserTextureFrame = 0;

if( GetFlashlightHandle() == CLIENTSHADOW_INVALID_HANDLE )
{
SetFlashlightHandle( g_pClientShadowMgr->CreateFlashlight( state ) );
}
else
{
g_pClientShadowMgr->UpdateFlashlightState( GetFlashlightHandle(), state );
}

g_pClientShadowMgr->UpdateProjectedTexture( GetFlashlightHandle(), true );
}

*/