//===== Copyright © 1996-2005, Valve Corporation, All rights reserved. ======//
//
// Purpose: 
//
//===========================================================================//

#include "cbase.h"
#include "c_muzzleflash_effect.h"
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

void r_newMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue);
//ConVar   weapon_muzzleflash_effect("weapon_muzzleflash_effect", "0");
static ConVar r_newM("r_newM", "1", FCVAR_CHEAT, "", r_newMCallback_f);
static ConVar r_swingM("r_swingM", "1", FCVAR_CHEAT);
static ConVar r_Mlockposition("r_Mlockposition", "0", FCVAR_CHEAT);
static ConVar r_MDynamicFov("r_MDynamicFov", "1");//Динамический угол обзора
static ConVar r_Mfov("r_Mfov", "130.0");//Угол обзора
static ConVar r_MfovTimer("r_MfovTimer", "10.0");//Счётчик для рандомного Fov-а
static ConVar r_MfovTimerBorder("r_MfovTimerBorder", "80.0");//Граница счётчика на рандомный Fov
static ConVar r_Moffsetx("r_Moffsetx", "0.0", FCVAR_CHEAT);//Положение по х
static ConVar r_Moffsety("r_Moffsety", "0.55", FCVAR_CHEAT);//Положение по у
static ConVar r_Moffsetz("r_Moffsetz", "-17.0", FCVAR_CHEAT);//Положение по z
static ConVar r_Mnear("r_Mnear", "4.0", FCVAR_CHEAT);
static ConVar r_Mfar("r_Mfar", "300.0");//Дальность
static ConVar r_Mconstant("r_Mconstant", "0.0", FCVAR_CHEAT);
static ConVar r_Mlinear("r_Mlinear", "100.0");//Яркость
static ConVar r_MDynamiclinear("r_MDynamiclinear", "0");//Динамическая яркость
static ConVar r_Mquadratic("r_Mquadratic", "0.0", FCVAR_CHEAT);
static ConVar r_Mvisualizetrace("r_Mvisualizetrace", "0", FCVAR_CHEAT);
static ConVar r_Mambient("r_Mambient", "0.0", FCVAR_CHEAT);
static ConVar r_Mshadowatten("r_Mshadowatten", "0.35", FCVAR_CHEAT);
static ConVar r_Mladderdist("r_Mladderdist", "40.0", FCVAR_CHEAT);
static ConVar mat_slopescaledepthbias_shadowmap_M("mat_slopescaledepthbias_shadowmap_M", "16", FCVAR_CHEAT);
static ConVar mat_depthbias_shadowmap_M("mat_depthbias_shadowmap_M", "0.0005", FCVAR_CHEAT);


void r_newMCallback_f(IConVar *pConVar, const char *pOldString, float flOldValue)
{
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newM.SetValue(0);
	}
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : nEntIndex - The m_nEntIndex of the client entity that is creating us.
//			vecPos - The position of the light emitter.
//			vecDir - The direction of the light emission.
//-----------------------------------------------------------------------------
C_MuzzleflashEffect::C_MuzzleflashEffect(int MnEntIndex)
{
	m_MHandle = CLIENTSHADOW_INVALID_HANDLE;
	m_nEntIndexM = MnEntIndex;

	m_M = false;

	m_pPointLightM = NULL;
	if (engine->GetDXSupportLevel() < 70)
	{
		r_newM.SetValue(0);
	}

	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	D = false;
	if (pPlayer)
	{
		//m_MTexture = new CTextureReference();

		int indexL = random->RandomInt(0, pPlayer->GetActiveWeapon()->MuzzleFlashTypeIndex());

		if (pPlayer->GetActiveWeapon()->MuzzleFlashType(indexL) != NULL)
			nameL = pPlayer->GetActiveWeapon()->MuzzleFlashType(indexL);

		if (nameL)
		{
			m_MTexture.Init(nameL, TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_MTexture.Init("effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
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

void C_MuzzleflashEffect::SetupTexture(C_BasePlayer *pPlayer)
{
	D = false;
	if (pPlayer)
	{
		/*m_MTexture->Shutdown();
		delete m_MTexture;
		m_MTexture = NULL;

		m_MTexture = new CTextureReference();*/

		int indexL = random->RandomInt(0, pPlayer->GetActiveWeapon()->MuzzleFlashTypeIndex());
		nameL = pPlayer->GetActiveWeapon()->MuzzleFlashType(indexL);

		if (nameL) 
		{
			m_MTexture.Init(nameL, TEXTURE_GROUP_OTHER, true);
		}
		else
		{
			m_MTexture.Init("effects/muzzleflash_light1", TEXTURE_GROUP_OTHER, true);
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
C_MuzzleflashEffect::~C_MuzzleflashEffect()
{
	LightOffM();
	//TimeTick = 0.0f;
	/*if (m_MTexture)
	{
		delete m_MTexture;
		m_MTexture = NULL;
	}*/
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::TurnOnM()
{
	m_M = true;
	m_flDistModM = 1.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::TurnOffM()
{
	/*if (m_M)
	{
		m_M = false;
		LightOffM();
		//TimeTick = 0.0f;
	}*/
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
void C_MuzzleflashEffect::UpdateLightNewM(const Vector &vecPos, const Vector &vecForward, const Vector &vecRight, const Vector &vecUp, float lKoef)
{
	VPROF_BUDGET("C_MuzzleflashEffect::UpdateLightNewM", VPROF_BUDGETGROUP_SHADOW_DEPTH_TEXTURING);

	FlashlightState_t state;

	// We will lock some of the flashlight params if player is on a ladder, to prevent oscillations due to the trace-rays
	bool bPlayerOnLadder = (C_BasePlayer::GetLocalPlayer()->GetMoveType() == MOVETYPE_LADDER);

	const float flEpsilon = 0.1f;			// Offset flashlight position along vecUp
	const float flDistCutoff = 128.0f;
	const float flDistDrag = 0.2;

	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	float MflOffsetY = r_Moffsety.GetFloat();
	float MflOffsetZ = r_Moffsetz.GetFloat();
	float MflOffsetX = r_Moffsetx.GetFloat();

	//    m_flMuzzleFlashRoll = RandomFloat( 0, 360.0f );

	if (r_swingM.GetBool())
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
	/////////////////////	Vector vOrigin = vecPos + flOffsetY * vecUp;
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

	Vector vTarget = vecPos + vecForward * r_Mfar.GetFloat();

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

	/*AssertFloatEquals(DotProduct(vDir, vRight), 2.5f, 1e-3);
	AssertFloatEquals(DotProduct(vDir, vUp), 0.3f, 1e-3);
	AssertFloatEquals(DotProduct(vRight, vUp), 0.0f, 1e-3);*/

	trace_t pmDirectionTrace;
	UTIL_TraceHull(vOrigin, vTarget, Vector(-4, -4, -4), Vector(-4, -4, -4), iMask, &traceFilter, &pmDirectionTrace);

	//	UTIL_Tracer( vOrigin, pmDirectionTrace.endpos, 0, TRACER_DONT_USE_ATTACHMENT, 5000, true, "GaussTracer" );

	if (r_Mvisualizetrace.GetBool() == true)
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
		float flPullBackDist = bPlayerOnLadder ? r_Mladderdist.GetFloat() : flDistCutoff - flDist;	// Fixed pull-back distance if on ladder
		m_flDistModM = Lerp(flDistDrag, m_flDistModM, flPullBackDist);

		if (!bPlayerOnLadder)
		{
			trace_t pmBackTrace;
			UTIL_TraceHull(vOrigin, vOrigin - vDir*(flPullBackDist - flEpsilon), Vector(-4, -4, -4), Vector(4, 4, 4), iMask, &traceFilter, &pmBackTrace);
			if (pmBackTrace.DidHit())
			{
				// We have an intersection behind us as well, so limit our m_flDistMod
				float flMaxDist = (pmBackTrace.endpos - vOrigin).Length() - flEpsilon;
				if (m_flDistModM > flMaxDist)
					m_flDistModM = flMaxDist;
			}
		}
	}
	else
	{
		m_flDistModM = Lerp(flDistDrag, m_flDistModM, 0.0f);
	}
	vOrigin = vOrigin - vDir * m_flDistModM;

	state.m_vecLightOrigin = vOrigin;

	BasisToQuaternion(vDir, vRight, vUp, state.m_quatOrientation);

	state.m_fQuadraticAtten = r_Mquadratic.GetFloat();

	bool bFlicker = false;

	if (r_MDynamicFov.GetInt())
	{
		float Timeing = RandomFloat(-10.0f, 10.0f);
		mFov = mFov + r_MfovTimer.GetFloat();
		if (mFov >= (r_MfovTimerBorder.GetFloat() + Timeing))
			mFov = 0.0f;

		if (bFlicker == false)
		{
			if (r_MDynamiclinear.GetInt())
			{
				int Rl1 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (lKoef * r_Mlinear.GetFloat() * Rl1);
			}
			if (!(r_MDynamiclinear.GetInt()))
				state.m_fLinearAtten = lKoef * r_Mlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_Mfov.GetFloat() - mFov);
			state.m_fVerticalFOVDegrees = (r_Mfov.GetFloat() - mFov);
		}

	}

	if (!(r_MDynamicFov.GetInt()))
	{
		int R = RandomFloat(-8, 8);
		if (bFlicker == false)
		{
			if (r_MDynamiclinear.GetInt())
			{
				int Rl2 = RandomFloat(0.1f, 8.0f);
				state.m_fLinearAtten = (lKoef * r_Mlinear.GetFloat() * Rl2);
			}
			if (!(r_MDynamiclinear.GetInt()))
				state.m_fLinearAtten = lKoef * r_Mlinear.GetFloat();

			state.m_fHorizontalFOVDegrees = (r_Mfov.GetFloat() - R);
			state.m_fVerticalFOVDegrees = (r_Mfov.GetFloat() - R);
		}
	}
	state.m_fConstantAtten = r_Mconstant.GetFloat();
	state.m_Color[0] = 1.0f;
	state.m_Color[1] = 1.0f;
	state.m_Color[2] = 1.0f;
	state.m_Color[3] = r_Mambient.GetFloat();
	state.m_NearZ = r_Mnear.GetFloat() + m_flDistModM;	// Push near plane out so that we don't clip the world when the flashlight pulls back 
	state.m_FarZ = r_Mfar.GetFloat();
	state.m_bEnableShadows = cvar->FindVar("r_flashlightShadows")->GetBool();//r_flashlightdepthtexture.GetBool();
	state.m_flShadowMapResolution = r_flashlightdepthres.GetInt();

	state.m_pSpotlightTexture = m_MTexture;
	//	state.m_pLaserTexture = m_LaserTexture;
	state.m_nSpotlightTextureFrame = 0;


	state.m_flShadowAtten = r_Mshadowatten.GetFloat();
	state.m_flShadowSlopeScaleDepthBias = mat_slopescaledepthbias_shadowmap_M.GetFloat();
	state.m_flShadowDepthBias = mat_depthbias_shadowmap_M.GetFloat();

	if (m_MHandle == CLIENTSHADOW_INVALID_HANDLE)
	{
		m_MHandle = g_pClientShadowMgr->CreateFlashlight(state);
	}
	else
	{
		if (!r_Mlockposition.GetBool())
		{
			g_pClientShadowMgr->UpdateFlashlightState(m_MHandle, state);
		}
	}

	g_pClientShadowMgr->UpdateProjectedTexture(m_MHandle, true);

	// Kill the old flashlight method if we have one.
	LightOffOldM();

#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Mentindex", m_nEntIndexM);
		msg->SetInt("MHandle", m_MHandle);
		msg->SetPtr("MState", &state);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	/*C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();

	if (!pPlayer)
		return;

	if (pPlayer->IsInAVehicle())
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
void C_MuzzleflashEffect::UpdateLightOldM(const Vector &vecPos, const Vector &vecDir, int nDistance, float lKoef)
{
	if (!m_pPointLightM || (m_pPointLightM->key != m_nEntIndexM))
	{
		// Set up the environment light
		m_pPointLightM = effects->CL_AllocDlight(m_nEntIndexM);
		m_pPointLightM->flags = 0.0f;
		m_pPointLightM->radius = 80;
	}

	// For bumped lighting
	VectorCopy(vecDir, m_pPointLightM->m_Direction);

	Vector end;
	end = vecPos + nDistance * vecDir;

	// Trace a line outward, skipping the player model and the view model.
	trace_t pm;
	CTraceFilterSkipPlayerAndViewModelSMG traceFilter;
	UTIL_TraceLine(vecPos, end, MASK_ALL, &traceFilter, &pm);
	VectorCopy(pm.endpos, m_pPointLightM->origin);

	float Mfalloff = pm.fraction * nDistance;

	if (Mfalloff < 500)
		Mfalloff = 1.0;
	else
		Mfalloff = 500.0 / Mfalloff;

	Mfalloff *= Mfalloff;

	m_pPointLightM->radius = 80;
	m_pPointLightM->color.r = m_pPointLightM->color.g = m_pPointLightM->color.b = 255 * Mfalloff;
	m_pPointLightM->color.exponent = 0;

	// Make it live for a bit
	m_pPointLightM->die = gpGlobals->curtime + 0.2f;

	// Update list of surfaces we influence
	render->TouchLight(m_pPointLightM);

	// kill the new flashlight if we have one
	LightOffNewM();
}

//-----------------------------------------------------------------------------
// Purpose: Do the headlight
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::UpdateLightM(const Vector &vecPos, const Vector &vecDir, const Vector &vecRight, const Vector &vecUp, int nDistance, float lKoef)
{
	if (!m_M)
	{
		return;
	}
	if (r_newM.GetBool())
	{
		UpdateLightNewM(vecPos, vecDir, vecRight, vecUp, lKoef);
	}
	else
	{
		UpdateLightOldM(vecPos, vecDir, nDistance, lKoef);
	}
}


//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::LightOffNewM()
{
#ifndef NO_TOOLFRAMEWORK
	if (clienttools->IsInRecordingMode())
	{
		KeyValues *msg = new KeyValues("MState");
		msg->SetFloat("time", gpGlobals->curtime);
		msg->SetInt("Mentindex", m_nEntIndexM);
		msg->SetInt("MHandle", m_MHandle);
		msg->SetPtr("MState", NULL);
		ToolFramework_PostToolMessage(HTOOLHANDLE_INVALID, msg);
		msg->deleteThis();
	}
#endif

	// Clear out the light
	if (m_MHandle != CLIENTSHADOW_INVALID_HANDLE)
	{
		g_pClientShadowMgr->DestroyFlashlight(m_MHandle);
		m_MHandle = CLIENTSHADOW_INVALID_HANDLE;
	}
	TimeTick = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::LightOffOldM()
{
	if (m_pPointLightM && (m_pPointLightM->key == m_nEntIndexM))
	{
		m_pPointLightM->die = gpGlobals->curtime;
		m_pPointLightM = NULL;
	}
	//TimeTick = 0.0f;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_MuzzleflashEffect::LightOffM()
{
	LightOffOldM();
	LightOffNewM();
	//TimeTick = 0.0f;
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