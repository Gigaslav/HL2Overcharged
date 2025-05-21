//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_Scope_Light.h"
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

void r_newScopeLightCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
static ConVar r_newScopeLight("r_newScopeLight", "1", FCVAR_CHEAT, "", r_newScopeLightCallback_f);
static ConVar r_swingScopeLight("r_swingScopeLight", "1", FCVAR_CHEAT);
static ConVar r_ScopeLightlockposition("r_ScopeLightlockposition", "0", FCVAR_CHEAT);
static ConVar r_ScopeLightDynamicFov("r_ScopeLightDynamicFov", "1", FCVAR_CHEAT);//Динамический угол обзора
static ConVar r_ScopeLightfov("r_ScopeLightfov", "130.0", FCVAR_CHEAT);//Угол обзора
static ConVar r_ScopeLightfov_adjust("r_ScopeLightfov_adjust", "0", FCVAR_CHEAT);//Угол обзора
static ConVar r_ScopeLightoffsetx("r_ScopeLightoffsetx", "0.0", FCVAR_CHEAT);//Положение по х
static ConVar r_ScopeLightoffsety("r_ScopeLightoffsety", "0.55", FCVAR_CHEAT);//Положение по у
static ConVar r_ScopeLightoffsetz("r_ScopeLightoffsetz", "-17.0", FCVAR_CHEAT);//Положение по z
static ConVar r_ScopeLightnear("r_ScopeLightnear", "4.0", FCVAR_CHEAT);
static ConVar r_ScopeLightfar("r_ScopeLightfar", "300.0", FCVAR_CHEAT);//Дальность
static ConVar r_ScopeLightconstant("r_ScopeLightconstant", "0.0", FCVAR_CHEAT);
static ConVar r_ScopeLightlinear("r_ScopeLightlinear", "100.0", FCVAR_CHEAT);//Яркость
static ConVar r_ScopeLightDynamiclinear("r_ScopeLightDynamiclinear", "0", FCVAR_CHEAT);//Динамическая яркость
static ConVar r_ScopeLightquadratic("r_ScopeLightquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_ScopeLightvisualizetrace("r_ScopeLightvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_ScopeLightambient("r_ScopeLightambient", "0.0", FCVAR_CHEAT);
static ConVar r_ScopeLightshadowatten("r_ScopeLightshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_ScopeLightladderdist("r_ScopeLightladderdist", "40.0", FCVAR_CHEAT);
static ConVar mat_slopescaledepthbias_shadowmap_ScopeLight("mat_slopescaledepthbias_shadowmap_ScopeLight", "16", FCVAR_CHEAT);
static ConVar mat_depthbias_shadowmap_ScopeLight("mat_depthbias_shadowmap_ScopeLight", "0.0005", FCVAR_CHEAT);


void r_newScopeLightCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newScopeLight.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_ScopeLight::C_ScopeLight(int ScopeLightnEntIndex, float _lightLinear, float constLight, float _lightFov, float _lightFar)
{
	m_ScopeLightHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndexScopeLight = ScopeLightnEntIndex;

	lightConstant = constLight;
	lightFov = _lightFov;
	lightFar = _lightFar;
	lightLinear = _lightLinear;

	m_ScopeLight = false;

	m_pPointLightScopeLight = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newScopeLight.SetValue(0);
	}


	if (g_pMaterialSystemHardwareConfig->SupportsBorderColor())
	{
		m_ScopeLightTexture.Init("effects/flashlight_border", TEXTURE_GROUP_OTHER, true);
	}
	else
	{
		m_ScopeLightTexture.Init("effects/flashlight001", TEXTURE_GROUP_OTHER, true);
	}

}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_ScopeLight::~C_ScopeLight()
{
	LightOffScopeLight();
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ScopeLight::TurnOnScopeLight()
{
	m_ScopeLight = true;
	m_flDistModScopeLight = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ScopeLight::TurnOffScopeLight()
{
	if (m_ScopeLight)
	{
		m_ScopeLight = false;
		LightOffScopeLight();
	}

}


// Custom trace filter that skips the player and the view model.
// If we don't do this, we'll end up having the light right in front of us all
// the time.
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
void C_ScopeLight::UpdateLightNewScopeLight(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp)
{
	VPROF_BUDGET("C_ScopeLight::UpdateLightNewScopeLight", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	float MflOffsetY = r_ScopeLightoffsety.GetFloat();
	float MflOffsetZ = r_ScopeLightoffsetz.GetFloat();
	float MflOffsetX = r_ScopeLightoffsetx.GetFloat();

	//    m_flMuzzleFlashRoll = RandomFloat( 0, 360.0f );

	if (r_swingScopeLight.GetBool())
	{
		// This projects the view direction backwards, attempting to raise the vertical
		// offset of the flashlight, but only when the player is looking down.
		Vector vecSwingLight = vecPos + vecForward * -12.0f;
		if (vecSwingLight.z > vecPos.z)
		{
			//		flOffsetY += (vecSwingLight.z - vecPos.z);//Вот оно////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		}
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

	Vector vTarget;
	if (r_ScopeLightfar.GetFloat() > 0)
		vTarget = vecPos + vecForward * r_ScopeLightfar.GetFloat();
	else
		vTarget = vecPos + vecForward * lightFar;

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

	if (r_ScopeLightvisualizetrace.GetBool() == true)
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
		float flPullBackDist = bPlayerOnLadder ? r_ScopeLightladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		m_flDistModScopeLight = Lerp(flDistDrag, m_flDistModScopeLight, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistModScopeLight > flMaxDist)
					m_flDistModScopeLight = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistModScopeLight = Lerp(flDistDrag, m_flDistModScopeLight, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistModScopeLight;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_ScopeLightquadratic.GetFloat();

	//bool bFlicker = false;
	if (r_ScopeLightfov_adjust.GetInt())
	{
		state.m_fHorizontalFOVDegrees = r_ScopeLightfov.GetInt();
		state.m_fVerticalFOVDegrees = r_ScopeLightfov.GetInt();
	}
	else
	{
		state.m_fHorizontalFOVDegrees = lightFov;
		state.m_fVerticalFOVDegrees = lightFov;
	}

	/*if (r_ScopeLightlinear.GetFloat() > 0)
		state.m_fLinearAtten = r_ScopeLightlinear.GetFloat();
	else*/
		state.m_fLinearAtten = lightLinear;
	
	/*if (r_ScopeLightconstant.GetFloat() > 0)
		state.m_fConstantAtten = r_ScopeLightconstant.GetFloat();
	else*/
		state.m_fConstantAtten = lightConstant;

	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_ScopeLightambient.GetFloat();
	state.m_NearZ = r_ScopeLightnear.GetFloat() + m_flDistModScopeLight;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	/*if (r_ScopeLightfar.GetFloat() > 0)
		state.m_FarZ = r_ScopeLightfar.GetFloat();
	else*/
		state.m_FarZ = lightFar;

	state.m_bEnableShadows = false;//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_ScopeLightTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;


	state.m_flShadowAtten = r_ScopeLightshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_ScopeLight.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_ScopeLight.GetFloat();

	if (m_ScopeLightHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_ScopeLightHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_ScopeLightlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_ScopeLightHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_ScopeLightHandle, true);

	// Kill the old flashlight method if we have one.
	LightOffOldScopeLight();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Mentindex", m_nEntIndexScopeLight);
		msg->SetInt("MHandle", m_ScopeLightHandle);
		msg->SetPtr("MState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_ScopeLight::UpdateLightOldScopeLight(const Vector &vecPos, const Vector &vecDir, int nDistance)
{
	if (!m_pPointLightScopeLight || (m_pPointLightScopeLight->key != m_nEntIndexScopeLight))
	{
		// Set up the environment light
		m_pPointLightScopeLight = effects->CL_AllocDlight(m_nEntIndexScopeLight);
		m_pPointLightScopeLight->flags = 0.0f;
		m_pPointLightScopeLight->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLightScopeLight->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLightScopeLight->origin);

	float Mfalloff = pm.fraction * nDistance;

	if (Mfalloff < 500)
		Mfalloff = 1.0;
	else
		Mfalloff = 500.0 / Mfalloff;

	Mfalloff *= Mfalloff;

	m_pPointLightScopeLight->radius = 80;
	m_pPointLightScopeLight->color.r = m_pPointLightScopeLight->color.g = m_pPointLightScopeLight->color.b = 255 * Mfalloff;
	m_pPointLightScopeLight->color.exponent = 0;

	// Make it live for a bit
	m_pPointLightScopeLight->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLightScopeLight);

	// kill the new flashlight if we have one
	LightOffNewScopeLight();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_ScopeLight::UpdateLightScopeLight(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance)
{
	if (!m_ScopeLight)
	{
		return;
	}
	if (r_newScopeLight.GetBool())
	{
		UpdateLightNewScopeLight(vecPos, vecDir, vecRight, vecUp);
	}
	else
	{
		UpdateLightOldScopeLight(vecPos, vecDir, nDistance);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ScopeLight::LightOffNewScopeLight()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Mentindex", m_nEntIndexScopeLight);
		msg->SetInt("MHandle", m_ScopeLightHandle);
		msg->SetPtr("MState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_ScopeLightHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_ScopeLightHandle);
		m_ScopeLightHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ScopeLight::LightOffOldScopeLight()
{
	if (m_pPointLightScopeLight && (m_pPointLightScopeLight->key == m_nEntIndexScopeLight))
	{
		m_pPointLightScopeLight->die = gpGlobals->curtime;
		m_pPointLightScopeLight = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_ScopeLight::LightOffScopeLight()
{
	LightOffOldScopeLight();
	LightOffNewScopeLight();
}
