//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_muzzleflash_laser_middle.h"
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

void r_newMLCallback_fM(IConVar *pConVar, const char *pOldString, float flOldValue);
//ConVar   weapon_muzzleflash_effect("weapon_muzzleflash_effect", "0");
static ConVar r_newMLM("r_newMLM", "1", FCVAR_CHEAT, "", r_newMLCallback_fM);
static ConVar r_swingMLM("r_swingMLM", "1", FCVAR_CHEAT);
static ConVar r_MLlockpositionM("r_MLlockpositionM", "0", FCVAR_CHEAT);
static ConVar r_MLDynamicFovM("r_MLDynamicFovM", "1");
static ConVar r_MLDynamicFovImm("r_MLDynamicFovImm", "1");//Imm
static ConVar r_MLfovM("r_MLfovM", "165.0");
static ConVar r_MLfovTimerM("r_MLfovTimerM", "20.0");
static ConVar r_MLfovTimerBorderM("r_MLfovTimerBorderM", "80.0");
static ConVar r_MLfovImm("r_MLfovImm", "115.0");//Imm
static ConVar r_MLfovTimerImm("r_MLfovTimerImm", "20.0");//Imm
static ConVar r_MLfovTimerBorderImm("r_MLfovTimerBorderImm", "80.0");//Imm
static ConVar r_MLoffsetxM("r_MLoffsetxM", "0.0");
static ConVar r_MLoffsetyM("r_MLoffsetyM", "0.55");
static ConVar r_MLoffsetzM("r_MLoffsetzM", "-17.0");
static ConVar r_MLnearM("r_MLnearM", "4.0");
static ConVar r_MLfarM("r_MLfarM", "600.0");
static ConVar r_MLfarImm("r_MLfarImm", "400.0");//Imm
static ConVar r_MLconstantM("r_MLconstantM", "0.0");
static ConVar r_MLconstantImm("r_MLconstantImm", "0.0");//Imm
static ConVar r_MLlinearM("r_MLlinearM", "1000.0");
static ConVar r_MLlinearImm("r_MLlinearImm", "1000.0");//Imm
static ConVar r_MLDynamiclinearM("r_MLDynamiclinearM", "0");
static ConVar r_MLDynamiclinearImm("r_MLDynamiclinearImm", "0");//Imm
static ConVar r_MLquadraticM("r_MLquadraticM", "0.0", FCVAR_CHEAT);
static ConVar r_MLvisualizetraceM("r_MLvisualizetraceM", "0", FCVAR_CHEAT);
static ConVar r_MLambientM("r_MLambientM", "0.0", FCVAR_CHEAT);
static ConVar r_MLshadowattenM("r_MLshadowattenM", "0.35", FCVAR_CHEAT);
static ConVar r_MLladderdistM("r_MLladderdistM", "40.0", FCVAR_CHEAT);
static ConVar mat_slopescaledepthbias_shadowmap_MLM("mat_slopescaledepthbias_shadowmap_MLM", "16", FCVAR_CHEAT);
static ConVar mat_depthbias_shadowmap_MLM("mat_depthbias_shadowmap_MLM", "0.0005", FCVAR_CHEAT);


void r_newMLCallback_fM(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newMLM.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_MuzzleflashLaserM::C_MuzzleflashLaserM(int MLMIndex)
{
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (!pPlayer)
		return;

	m_MLMHandle = CLIENTSHADOW_INVALID_HANDLE;
	IndexMLM = MLMIndex;


	m_MLM = false;

	m_pPointLightMLM = NULL;

	if (pPlayer->IsEffectActive(EF_IMM))
		Switcher = false;

	if (pPlayer->IsEffectActive(EF_ML))
		Switcher = true;

	if (engine->GetDXSupportLevel() < 70)
	{
		r_newMLM.SetValue(0);
	}

	if (Switcher)
	{
		if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
		{
			m_MLMTexture.Init("effects/LaserMiddle", TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_MLMTexture.Init("effects/LaserMiddle", TEXTURE_GROUP_OTHER, true);
		}
	}
	else if (!Switcher)
	{
		if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
		{
			m_MLMTexture.Init("effects/ImmolatorMiddle", TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_MLMTexture.Init("effects/ImmolatorMiddle", TEXTURE_GROUP_OTHER, true);
		}
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_MuzzleflashLaserM::~C_MuzzleflashLaserM()
{
	LightOffMLM();

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::TurnOnMLM()
{
	m_MLM = true;
	m_flDistModMLM = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::TurnOffMLM()
{
	if (m_MLM)
	{
		m_MLM = false;
		LightOffMLM();
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
void C_MuzzleflashLaserM::UpdateLightNewMLM(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	VPROF_BUDGET("C_MuzzleflashEffectM::UpdateLightNewMLM", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	float MflOffsetY = r_MLoffsetyM.GetFloat();
	float MflOffsetZ = r_MLoffsetzM.GetFloat();
	float MflOffsetX = r_MLoffsetxM.GetFloat();


	if (r_swingMLM.GetBool())
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

	Vector vTarget = vecPos + vecForward * r_MLfarM.GetFloat();

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

	if (r_MLvisualizetraceM.GetBool() == true)
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
		float flPullBackDist = bPlayerOnLadder ? r_MLladderdistM.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		m_flDistModMLM = Lerp(flDistDrag, m_flDistModMLM, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistModMLM > flMaxDist)
					m_flDistModMLM = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistModMLM = Lerp(flDistDrag, m_flDistModMLM, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistModMLM;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_MLquadraticM.GetFloat();

	bool bFlicker = false;

	if (r_MLDynamicFovM.GetInt())
	{
		float Timeing = RandomFloat(-10.0f, 10.0f);
		mFov = mFov + r_MLfovTimerM.GetFloat();
		if (mFov >= (r_MLfovTimerBorderM.GetFloat() + Timeing))
			mFov = 0.0f;

		if (bFlicker == false)
		{
			if (r_MLDynamiclinearM.GetInt())
			{
				int Rl1 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (r_MLlinearM.GetFloat() * Rl1);
			}
			if (!(r_MLDynamiclinearM.GetInt()))
				state.m_fLinearAtten = r_MLlinearM.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_MLfovM.GetFloat() - mFov);
			state.m_fVerticalFOVDegrees = (r_MLfovM.GetFloat() - mFov);
		}
	}
	if (!(r_MLDynamicFovM.GetInt()))
	{
		if (Switcher)
		{
			int R = RandomFloat(-8, 8);
			if (bFlicker == false)
			{
				if (r_MLDynamiclinearM.GetInt())
				{
					int Rl2 = RandomFloat(0.1f, 8.0f);
					state.m_fLinearAtten = (r_MLlinearM.GetFloat() * Rl2);
				}
				if (!(r_MLDynamiclinearM.GetInt()))
					state.m_fLinearAtten = r_MLlinearM.GetFloat();

				state.m_fHorizontalFOVDegrees = (r_MLfovM.GetFloat() - R);
				state.m_fVerticalFOVDegrees = (r_MLfovM.GetFloat() - R);
			}
		}
	}
	if (!(r_MLDynamicFovImm.GetInt()))
	{
		if (!Switcher)
		{
			int R = RandomFloat(-8, 8);
			if (bFlicker == false)
			{
				if (r_MLDynamiclinearImm.GetInt())
				{
					int Rl2 = RandomFloat(0.1f, 8.0f);
					state.m_fLinearAtten = (r_MLlinearImm.GetFloat() * Rl2);
				}
				if (!(r_MLDynamiclinearImm.GetInt()))
					state.m_fLinearAtten = r_MLlinearImm.GetFloat();

				state.m_fHorizontalFOVDegrees = (r_MLfovImm.GetFloat() - R);
				state.m_fVerticalFOVDegrees = (r_MLfovImm.GetFloat() - R);
			}
		}
	}


	state.m_fConstantAtten = r_MLconstantM.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_MLambientM.GetFloat();
	state.m_NearZ = r_MLnearM.GetFloat() + m_flDistModMLM;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_MLfarM.GetFloat();
	state.m_bEnableShadows = cvar->FindVar("r_flashlightShadows")->GetBool();//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_MLMTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;


	state.m_flShadowAtten = r_MLshadowattenM.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_MLM.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_MLM.GetFloat();

	if (m_MLMHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_MLMHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_MLlockpositionM.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_MLMHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_MLMHandle, true);

	// Kill the old flashlight method if we have one.
	LightOffOldMLM();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MLState");
		msg->SetFloat("Ltime", gpGlobals->curtime);
		msg->SetInt("MLMentindex", IndexMLM);
		msg->SetInt("MLMHandle", m_MLMHandle);
		msg->SetPtr("MLState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::UpdateLightOldMLM(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_pPointLightMLM || (m_pPointLightMLM->key != IndexMLM))
	{
		// Set up the environment light
		m_pPointLightMLM = effects->CL_AllocDlight(IndexMLM);
		m_pPointLightMLM->flags = 0.0f;
		m_pPointLightMLM->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLightMLM->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLightMLM->origin);

	float Mfalloff = pm.fraction * nDistance;

	if (Mfalloff < 500)
		Mfalloff = 1.0;
	else
		Mfalloff = 500.0 / Mfalloff;

	Mfalloff *= Mfalloff;

	m_pPointLightMLM->radius = 80;
	m_pPointLightMLM->color.r = m_pPointLightMLM->color.g = m_pPointLightMLM->color.b = 255 * Mfalloff;
	m_pPointLightMLM->color.exponent = 0;

	// Make it live for a bit
	m_pPointLightMLM->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLightMLM);

	// kill the new flashlight if we have one
	LightOffNewMLM();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::UpdateLightMLM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
{
	if (!m_MLM)
	{
		return;
	}
	if (r_newMLM.GetBool())
	{
		UpdateLightNewMLM(vecPos, vecDir, vecRight, vecUp);
	}
	else
	{
		UpdateLightOldMLM(vecPos, vecDir, nDistance);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::LightOffNewMLM()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MLState");
		msg->SetFloat("Ltime", gpGlobals->curtime);
		msg->SetInt("MLMentindex", IndexMLM);
		msg->SetInt("MLMHandle", m_MLMHandle);
		msg->SetPtr("MLState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_MLMHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_MLMHandle);
		m_MLMHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::LightOffOldMLM()
{
	if (m_pPointLightMLM && (m_pPointLightMLM->key == IndexMLM))
	{
		m_pPointLightMLM->die = gpGlobals->curtime;
		m_pPointLightMLM = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashLaserM::LightOffMLM()
{
	LightOffOldMLM();
	LightOffNewMLM();
}
