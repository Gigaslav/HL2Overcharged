//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_muzzleflash_laser.h"
#include "dlight.h"
#include "iefx.h"
#include "iviewrender.h"
#include "view.h"
#include "engine/ivdebugoverlay.h"
#include "tier0/vprof.h"
#include "tier1/KeyValues.h"
#include "toolframework_client.h"

#include "const.h"
#include "view_shared.h"
#include "view.h"
#include "baseviewmodel_shared.h"///

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

void r_newMLCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
//ConVar   weapon_muzzleflash_effect("weapon_muzzleflash_effect", "0");
static ConVar r_newML("r_newML", "1", FCVAR_CHEAT, "", r_newMLCallback_f);
static ConVar r_swingML("r_swingML", "1", FCVAR_CHEAT);
static ConVar r_MLlockposition("r_MLlockposition", "0", FCVAR_CHEAT);
static ConVar r_MLDynamicFov("r_MLDynamicFov", "1", FCVAR_CHEAT);
static ConVar r_MLfov("r_MLfov", "115.0");
static ConVar r_MLfovTimer("r_MLfovTimer", "20.0");
static ConVar r_MLfovTimerBorder("r_MLfovTimerBorder", "80.0");
static ConVar r_MLoffsetx("r_MLoffsetx", "0.0", FCVAR_CHEAT);
static ConVar r_MLoffsety("r_MLoffsety", "0.55", FCVAR_CHEAT);
static ConVar r_MLoffsetz("r_MLoffsetz", "-17.0", FCVAR_CHEAT);
static ConVar r_MLnear("r_MLnear", "4.0", FCVAR_CHEAT);
static ConVar r_MLfar("r_MLfar", "400.0");
static ConVar r_MLconstant("r_MLconstant", "0.0", FCVAR_CHEAT);
static ConVar r_MLlinear("r_MLlinear", "100.0");
static ConVar r_MLDynamiclinear("r_MLDynamiclinear", "0");
static ConVar r_MLquadratic("r_MLquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_MLvisualizetrace("r_MLvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_MLambient("r_MLambient", "0.0", FCVAR_CHEAT);
static ConVar r_MLshadowatten("r_MLshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_MLladderdist("r_MLladderdist", "40.0", FCVAR_CHEAT);
static ConVar mat_slopescaledepthbias_shadowmap_ML("mat_slopescaledepthbias_shadowmap_ML", "16", FCVAR_CHEAT);
static ConVar mat_depthbias_shadowmap_ML("mat_depthbias_shadowmap_ML", "0.0005", FCVAR_CHEAT);


void r_newMLCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newML.SetValue(0);
	}
}

float SpeedOfTime;
float MaxTime;
float TimeTick = 0.0f;
//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_MuzzleflashLaser::C_MuzzleflashLaser(int MLIndex)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_MLHandle = CLIENTSHADOW_INVALID_HANDLE;
	IndexML = MLIndex;


	m_ML = false;

//	int j = random->RandomInt(1, 4);

	if (pPlayer->IsEffectActive(EF_IMM))
	Switcher = false;

	if (pPlayer->IsEffectActive(EF_ML))
	Switcher = true;

	m_pPointLightML = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newML.SetValue(0);
	}

	/*if (Switcher)
	{
		if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
		{
			m_MLTexture.Init("effects/red_light1", TEXTURE_GROUP_OTHER, true);
		}
		else
		{

			if (j == 1)
			{
				m_MLTexture.Init("effects/red_light1", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 2)
			{
				m_MLTexture.Init("effects/red_light2", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 3)
			{
				m_MLTexture.Init("effects/red_light3", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 4)
			{
				m_MLTexture.Init("effects/red_light4", TEXTURE_GROUP_OTHER, true);
			}
		}
	}
	else if (!Switcher)
	{
		if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
		{
			m_MLTexture.Init("effects/immolator_flash1", TEXTURE_GROUP_OTHER, true);
		}
		else
		{

			if (j == 1)
			{
				m_MLTexture.Init("effects/immolator_flash1", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 2)
			{
				m_MLTexture.Init("effects/immolator_flash2", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 3)
			{
				m_MLTexture.Init("effects/immolator_flash3", TEXTURE_GROUP_OTHER, true);
			}
			if (j == 4)
			{
				m_MLTexture.Init("effects/immolator_flash4", TEXTURE_GROUP_OTHER, true);
			}
		}
	}*/


	/*C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	//CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (!pPlayer)
		return;*/

	if (pPlayer)
	{
		IndexML = pPlayer->GetActiveWeapon()->FlashType();

		switch (IndexML)
		{
		case 1:
			m_MLTexture.Init("effects/armuzzlelight0001", TEXTURE_GROUP_OTHER, true);//
			break;
		case 2:
			m_MLTexture.Init("effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
			break;
		case 3:
			m_MLTexture.Init("effects/red_light1", TEXTURE_GROUP_OTHER, true);
			break;
		case 4:
			m_MLTexture.Init("effects/immolator_flash1", TEXTURE_GROUP_OTHER, true);
			break;
		default:
			break;
		}
		if (IndexML == NULL)
		{
			m_MLTexture.Init("effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
		}


		//TimeTick = 0.0f;
		SpeedOfTime = cvar->FindVar("oc_ProjectedMuzzleFlash_SpeedOfTime")->GetFloat();
		if (cvar->FindVar("oc_ProjectedMuzzleFlash_test")->GetInt())
			MaxTime = cvar->FindVar("oc_ProjectedMuzzleFlash_LifeTime")->GetFloat();
		else
		{
			if (pPlayer->GetActiveWeapon()->FlashLifeTime() == NULL)
				MaxTime = cvar->FindVar("oc_ProjectedMuzzleFlash_LifeTime")->GetFloat();
			else
				MaxTime = pPlayer->GetActiveWeapon()->FlashLifeTime();
		}
	}
	TimeTick = 0.0f;
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_MuzzleflashLaser::~C_MuzzleflashLaser()
{
	LightOffML();

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::TurnOnML()
{
	m_ML = true;
	m_flDistModML = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::TurnOffML()
{
	if (m_ML)
	{
		m_ML = false;
		LightOffML();
	}

}


class CTraceFilterSkipPlayerAndViewModelSMG : public CTraceFilter
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
void C_MuzzleflashLaser::UpdateLightNewML(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	VPROF_BUDGET("C_MuzzleflashEffect::UpdateLightNewML", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	float MflOffsetY = r_MLoffsety.GetFloat();
	float MflOffsetZ = r_MLoffsetz.GetFloat();
	float MflOffsetX = r_MLoffsetx.GetFloat();


	if (r_swingML.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
	}

	Vector vOrigin = vecPos + ((MflOffsetZ * vecForward) + MflOffsetX * vecUp + (MflOffsetY * vecRight));

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

	Vector vTarget = vecPos + vecForward * r_MLfar.GetFloat();

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

	AssertFloatEquals(DotProduct(vDir, vRight), 2.5f, 1e-3);
	AssertFloatEquals(DotProduct(vDir, vUp), 0.3f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);

	trace_t pmDirectionTrace;
	UTIL_TraceHull(vOrigin, vTarget, Vector(-4, -4, -4), Vector(-4, -4, -4), iMask, &traceFilter, &pmDirectionTrace);

	//	UTIL_Tracer( vOrigin, pmDirectionTrace.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 5000, true, "GaussTracer" );

	if (r_MLvisualizetrace.GetBool() == true)
	{
		debugoverlay->AddBoxOverlay(pmDirectionTrace.endpos, Vector(-4, -4, -4), Vector(4, 4, 4), QAngle(0, 0, 0), 0, 0, 255, 16, 0);
		debugoverlay->AddLineOverlay(vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0);
		//		UTIL_Tracer( vOrigin, pmDirectionTrace.endpos, 0, TRACER_FLAG_USEATTACHMENT, 4000, false, "CguardTracer" );
	}

	float flDist = (pmDirectionTrace.endpos - vOrigin).Length();
	if (flDist < flDistCutoff)
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_MLladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		m_flDistModML = Lerp(flDistDrag, m_flDistModML, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistModML > flMaxDist)
					m_flDistModML = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistModML = Lerp(flDistDrag, m_flDistModML, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistModML;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_MLquadratic.GetFloat();

	bool bFlicker = false;

	if (r_MLDynamicFov.GetInt())
	{
		float Timeing = RandomFloat(-10.0f, 10.0f);
		mFov = mFov + r_MLfovTimer.GetFloat();
		if (mFov >= (r_MLfovTimerBorder.GetFloat() + Timeing))
			mFov = 0.0f;

		if (bFlicker == false)
		{
     		if (r_MLDynamiclinear.GetInt())
			{
				int Rl1 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (r_MLlinear.GetFloat() * Rl1);
			}
			if (!(r_MLDynamiclinear.GetInt()))
				state.m_fLinearAtten = r_MLlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_MLfov.GetFloat() - mFov);
			state.m_fVerticalFOVDegrees = (r_MLfov.GetFloat() - mFov);
		}
	}
	if (!(r_MLDynamicFov.GetInt()))
	{
		int R = RandomFloat(-8, 8);
		if (bFlicker == false)
		{
			if (r_MLDynamiclinear.GetInt())
			{
				int Rl2 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (r_MLlinear.GetFloat() * Rl2);
			}
			if (!(r_MLDynamiclinear.GetInt()))
				state.m_fLinearAtten = r_MLlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_MLfov.GetFloat() - R);
			state.m_fVerticalFOVDegrees = (r_MLfov.GetFloat() - R);
		}
	}

	state.m_fConstantAtten = r_MLconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_MLambient.GetFloat();
	state.m_NearZ = r_MLnear.GetFloat() + m_flDistModML;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_MLfar.GetFloat();
	state.m_bEnableShadows = cvar->FindVar("r_flashlightShadows")->GetBool();//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_MLTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;


	state.m_flShadowAtten = r_MLshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_ML.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_ML.GetFloat();

	if (m_MLHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_MLHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_MLlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_MLHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_MLHandle, true);

	// Kill the old flashlight method if we have one.
	LightOffOldML();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MLState");
		msg->SetFloat("Ltime", gpGlobals->curtime);
		msg->SetInt("MLentindex", IndexML);
		msg->SetInt("MLHandle", m_MLHandle);
		msg->SetPtr("MLState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	/*C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	//CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	if (!pPlayer)
		return;
	//DevMsg("1: %.2f \n", 1);
	if (pPlayer->IsInAVehicle())
		pPlayer->RemoveEffects(EF_ML);

	if (TimeTick < MaxTime)
	{
		TimeTick = TimeTick + SpeedOfTime *gpGlobals->frametime;
	}
	else if (TimeTick >= MaxTime)
	{
		//TimeTick = 0.0f;
		pPlayer->RemoveEffects(EF_ML);
		TimeTick = 0.0f;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::UpdateLightOldML(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_pPointLightML || (m_pPointLightML->key != IndexML))
	{
		// Set up the environment light
		m_pPointLightML = effects->CL_AllocDlight(IndexML);
		m_pPointLightML->flags = 0.0f;
		m_pPointLightML->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLightML->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLightML->origin);

	float Mfalloff = pm.fraction * nDistance;

	if (Mfalloff < 500)
		Mfalloff = 1.0;
	else
		Mfalloff = 500.0 / Mfalloff;

	Mfalloff *= Mfalloff;

	m_pPointLightML->radius = 80;
	m_pPointLightML->color.r = m_pPointLightML->color.g = m_pPointLightML->color.b = 255 * Mfalloff;
	m_pPointLightML->color.exponent = 0;

	// Make it live for a bit
	m_pPointLightML->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLightML);

	// kill the new flashlight if we have one
	LightOffNewML();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::UpdateLightML(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
{
	if (!m_ML)
	{
		return;
	}
	if (r_newML.GetBool())
	{
		UpdateLightNewML(vecPos, vecDir, vecRight, vecUp);
	}
	else
	{
		UpdateLightOldML(vecPos, vecDir, nDistance);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::LightOffNewML()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MLState");
		msg->SetFloat("Ltime", gpGlobals->curtime);
		msg->SetInt("MLentindex", IndexML);
		msg->SetInt("MLHandle", m_MLHandle);
		msg->SetPtr("MLState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_MLHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_MLHandle);
		m_MLHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::LightOffOldML()
{
	if (m_pPointLightML && (m_pPointLightML->key == IndexML))
	{
		m_pPointLightML->die = gpGlobals->curtime;
		m_pPointLightML = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaser::LightOffML()
{
	LightOffOldML();
	LightOffNewML();
}
