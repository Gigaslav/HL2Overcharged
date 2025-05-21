//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_ar2_middle_muzzleflash_effect.h"
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

void r_MnewMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
//ConVar   weapon_muzzleflash_effect("weapon_muzzleflash_effect", "0");
static ConVar r_MMnew("r_MMnew", "1", FCVAR_CHEAT, "", r_MnewMCallback_f);
static ConVar r_MMswing("r_MMswing", "1", FCVAR_CHEAT);
static ConVar r_MMlockposition("r_MMlockposition", "0", FCVAR_CHEAT);
static ConVar r_MMDynamicFov("r_MMDynamicFov", "1");
static ConVar r_MMfov("r_MMfov", "150.0");
static ConVar r_MMfovTimer("r_MMfovTimer", "10.0");
static ConVar r_MMfovTimerBorder("r_MMfovTimerBorder", "80.0");
static ConVar r_MMoffsetx("r_MMoffsetx", "0.0", FCVAR_CHEAT);
static ConVar r_MMoffsety("r_MMoffsety", "0.55", FCVAR_CHEAT);
static ConVar r_MMoffsetz("r_MMoffsetz", "-17.0", FCVAR_CHEAT);
static ConVar r_MMnear("r_MMnear", "4.0", FCVAR_CHEAT);
static ConVar r_MMfar("r_MMfar", "300.0");
static ConVar r_MMconstant("r_MMconstant", "0.0", FCVAR_CHEAT);
static ConVar r_MMlinear("r_MMlinear", "10");
static ConVar r_MMDynamiclinear("r_MMDynamiclinear", "0", FCVAR_CHEAT);
static ConVar r_MMquadratic("r_MMquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_MMvisualizetrace("r_MMvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_MMambient("r_MMambient", "0.0", FCVAR_CHEAT);
static ConVar r_MMshadowatten("r_MMshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_MMladderdist("r_MMladderdist", "40.0", FCVAR_CHEAT);
static ConVar mat_slopescaledepthbias_shadowmap_MM("mat_slopescaledepthbias_shadowmap_MM", "16", FCVAR_CHEAT);
static ConVar mat_depthbias_shadowmap_MM("mat_depthbias_shadowmap_MM", "0.0005", FCVAR_CHEAT);


void r_MnewMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_MMnew.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_Ar2_Middle_MuzzleflashEffect::C_Ar2_Middle_MuzzleflashEffect(int AR2_MnEntIndex)
{
	m_Ar2_MMHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_Ar2_MnEntIndexM = AR2_MnEntIndex;

	m_Ar2_MM = false;

	m_Ar2_MpPointLightM = NULL;

	if (engine->GetDXSupportLevel() < 70)
	{
		r_MMnew.SetValue(0);
	}

	//CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	if (pPlayer)
	{
		//m_Ar2_MMTexture = new CTextureReference();

		int indexL = random->RandomInt(0, pPlayer->GetActiveWeapon()->MuzzleSecondaryFlashTypeIndex());

		if (pPlayer->GetActiveWeapon()->MuzzleFlashType(indexL) != NULL)
			nameL = pPlayer->GetActiveWeapon()->MuzzleSecondaryFlashType(indexL);

		if (nameL)
		{
			m_Ar2_MMTexture.Init(nameL, TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_Ar2_MMTexture.Init("effects/ar2muzzleflash_light2", TEXTURE_GROUP_OTHER, true);
		}

		//SpeedOfTime = cvar->FindVar("oc_ProjectedMuzzleFlash_SpeedOfTime")->GetFloat();
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
}

void C_Ar2_Middle_MuzzleflashEffect::SetupTexture(C_BasePlayer *pPlayer)
{
	if (pPlayer)
	{
		/*m_Ar2_MMTexture->Shutdown();
		delete m_Ar2_MMTexture;
		m_Ar2_MMTexture = NULL;

		m_Ar2_MMTexture = new CTextureReference();*/

		int indexL = random->RandomInt(0, pPlayer->GetActiveWeapon()->MuzzleFlashTypeIndex());
		nameL = pPlayer->GetActiveWeapon()->MuzzleFlashType(indexL);

		if (nameL)
		{
			m_Ar2_MMTexture.Init(nameL, TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_Ar2_MMTexture.Init("effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
		}

		//SpeedOfTime = cvar->FindVar("oc_ProjectedMuzzleFlash_SpeedOfTime")->GetFloat();
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
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Ar2_Middle_MuzzleflashEffect::~C_Ar2_Middle_MuzzleflashEffect()
{
	Ar2_Middle_LightOffM();

	/*if (m_Ar2_MMTexture)
	{
		delete m_Ar2_MMTexture;
		m_Ar2_MMTexture = NULL;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_TurnOnM()
{
	m_Ar2_MM = true;
	m_Ar2_MflDistModM = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_TurnOffM()
{
	/*if (m_Ar2_MM)
	{
		m_Ar2_MM = false;
		Ar2_Middle_LightOffM();
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
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_UpdateLightNewM(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, float lKoef)
{
	VPROF_BUDGET("C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_UpdateLightNewM", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float Ar2_MflEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float Ar2_MflDistCutoff = 128.0f;
	const float Ar2_MflDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelAR2 traceFilter;
	float Ar2_MflOffsetY = r_MMoffsety.GetFloat();
	float Ar2_MflOffsetZ = r_MMoffsetz.GetFloat();
	float Ar2_MflOffsetX = r_MMoffsetx.GetFloat();

	//    m_flMuzzleFlashRoll = RandomFloat( 0, 360.0f );

	if (r_MMswing.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			//		flOffsetY += (vecSwingLight.z - vecPos.z);//Вот оно////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
	}

	Vector vOrigin = vecPos + ((Ar2_MflOffsetZ * vecForward) + Ar2_MflOffsetX * vecUp + (Ar2_MflOffsetY * vecRight));
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

	Vector vTarget = vecPos + vecForward * r_MMfar.GetFloat();

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

	if (r_MMvisualizetrace.GetBool() == true)
	{
		debugoverlay->AddBoxOverlay(pmDirectionTrace.endpos, Vector(-4, -4, -4), Vector(4, 4, 4), QAngle(0, 0, 0), 0, 0, 255, 16, 0);
		debugoverlay->AddLineOverlay(vOrigin, pmDirectionTrace.endpos, 255, 0, 0, false, 0);
	}

	float Ar2_flDist = (pmDirectionTrace.endpos - vOrigin).Length();
	if (Ar2_flDist < Ar2_MflDistCutoff)
	{
		// We have an intersection with our cutoff range
		// Determine how far to pull back, then trace to see if we are clear
		float flPullBackDist = bPlayerOnLadder ? r_MMladderdist.GetFloat() : Ar2_MflDistCutoff - Ar2_flDist;	// Fixed pull-back distance if on ladder
		m_Ar2_MflDistModM = Lerp(Ar2_MflDistDrag, m_Ar2_MflDistModM, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - Ar2_MflEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - Ar2_MflEpsilon;
				if (m_Ar2_MflDistModM > flMaxDist)
					m_Ar2_MflDistModM = flMaxDist;
			}
		}
	}
	else
	{
		m_Ar2_MflDistModM = Lerp(Ar2_MflDistDrag, m_Ar2_MflDistModM, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_Ar2_MflDistModM;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_MMquadratic.GetFloat();

	bool bFlicker = false;



	//CBasePlayer *pPlayer = UTIL_PlayerByIndex(engine->GetLocalPlayer());
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	//if (pPlayer->GetActiveWeapon()->FlashType() != 3 || pPlayer->GetActiveWeapon()->FlashType() != 4)
	{
		if (r_MMDynamicFov.GetInt())
		{
			float Timeing = RandomFloat(-10.0f, 10.0f);
			mFov = mFov + r_MMfovTimer.GetFloat();
			if (mFov >= (r_MMfovTimerBorder.GetFloat() + Timeing))
				mFov = 0.0f;

			if (bFlicker == false)
			{
				if (r_MMDynamiclinear.GetInt())
				{
					int Rl1 = RandomFloat(0.1f, 8.0f);
					state.m_fLinearAtten = (lKoef * r_MMlinear.GetFloat() * Rl1);
				}
				if (!(r_MMDynamiclinear.GetInt()))
					state.m_fLinearAtten = lKoef * r_MMlinear.GetFloat();

				state.m_fHorizontalFOVDegrees = (r_MMfov.GetFloat() - mFov);
				state.m_fVerticalFOVDegrees = (r_MMfov.GetFloat() - mFov);
			}
		}
		if (!(r_MMDynamicFov.GetInt()))
		{
			int R = RandomFloat(-8, 8);
			if (bFlicker == false)
			{
				if (r_MMDynamiclinear.GetInt())
				{
					int Rl2 = RandomFloat(0.1f, 8.0f);
					state.m_fLinearAtten = (lKoef * r_MMlinear.GetFloat() * Rl2);
				}
				if (!(r_MMDynamiclinear.GetInt()))
					state.m_fLinearAtten = lKoef * r_MMlinear.GetFloat();

				state.m_fHorizontalFOVDegrees = (r_MMfov.GetFloat() - R);
				state.m_fVerticalFOVDegrees = (r_MMfov.GetFloat() - R);
			}
		}
	}

	state.m_fConstantAtten = r_MMconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_MMambient.GetFloat();
	state.m_NearZ = r_MMnear.GetFloat() + m_Ar2_MflDistModM;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_MMfar.GetFloat();
	state.m_bEnableShadows = cvar->FindVar("r_flashlightShadows")->GetBool();//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_Ar2_MMTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;
	//	state.m_nSpotLaserTextureFrame = 0;


	state.m_flShadowAtten = r_MMshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_MM.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_MM.GetFloat();

	if (m_Ar2_MMHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_Ar2_MMHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_MMlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_Ar2_MMHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_Ar2_MMHandle, true);

	// Kill the old flashlight method if we have one.
	Ar2_Middle_LightOffOldM();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("Ar2_MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Ar2_entindex", m_Ar2_MnEntIndexM);
		msg->SetInt("Ar2_MHandle", m_Ar2_MMHandle);
		msg->SetPtr("Ar2_MState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	/*if (pPlayer->IsInAVehicle())
		pPlayer->RemoveEffects(EF_M);

	if (TimeTick < MaxTime)
	{
		TimeTick = TimeTick + SpeedOfTime *gpGlobals->frametime;
	}
	else if (TimeTick >= MaxTime)
	{
		pPlayer->RemoveEffects(EF_M);
		TimeTick = 0.0f;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance, float lKoef)
{
	if (!m_Ar2_MpPointLightM || (m_Ar2_MpPointLightM->key != m_Ar2_MnEntIndexM))
	{
		// Set up the environment light
		m_Ar2_MpPointLightM = effects->CL_AllocDlight(m_Ar2_MnEntIndexM);
		m_Ar2_MpPointLightM->flags = 0.0f;
		m_Ar2_MpPointLightM->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_Ar2_MpPointLightM->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelAR2 traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_Ar2_MpPointLightM->origin);

	float Ar2_falloff = pm.fraction * nDistance;

	if (Ar2_falloff < 500)
		Ar2_falloff = 1.0;
	else
		Ar2_falloff = 500.0 / Ar2_falloff;

	Ar2_falloff *= Ar2_falloff;

	m_Ar2_MpPointLightM->radius = 80;
	m_Ar2_MpPointLightM->color.r = m_Ar2_MpPointLightM->color.g = m_Ar2_MpPointLightM->color.b = 255 * Ar2_falloff;
	m_Ar2_MpPointLightM->color.exponent = 0;

	// Make it live for a bit
	m_Ar2_MpPointLightM->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_Ar2_MpPointLightM);

	// kill the new flashlight if we have one
	Ar2_Middle_LightOffNewM();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, float lKoef)
{
	if (!m_Ar2_MM)
	{
		return;
	}
	if (r_MMnew.GetBool())
	{
		Ar2_Middle_UpdateLightNewM(vecPos, vecDir, vecRight, vecUp, lKoef);
	}
	else
	{
		Ar2_Middle_UpdateLightOldM(vecPos, vecDir, nDistance, lKoef);
	}

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_LightOffNewM()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("Ar2_MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Ar2_Mentindex", m_Ar2_MnEntIndexM);
		msg->SetInt("Ar2_MMHandle", m_Ar2_MMHandle);
		msg->SetPtr("Ar2_MState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_Ar2_MMHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_Ar2_MMHandle);
		m_Ar2_MMHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_LightOffOldM()
{
	if (m_Ar2_MpPointLightM && (m_Ar2_MpPointLightM->key == m_Ar2_MnEntIndexM))
	{
		m_Ar2_MpPointLightM->die = gpGlobals->curtime;
		m_Ar2_MpPointLightM = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Ar2_Middle_MuzzleflashEffect::Ar2_Middle_LightOffM()
{
	Ar2_Middle_LightOffOldM();
	Ar2_Middle_LightOffNewM();
}
