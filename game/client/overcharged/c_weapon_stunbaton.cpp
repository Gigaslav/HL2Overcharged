//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: 
//
//=============================================================================//
#include "cbase.h"
#include "c_basehlcombatweapon.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "c_weapon__stubs.h"
#include "materialsystem/imaterial.h"
#include "clienteffectprecachesystem.h"
#include "beamdraw.h"
#include "dlight.h"
#include "r_efx.h"
#include "c_te_effect_dispatch.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectStunbaton)
CLIENTEFFECT_MATERIAL("effects/stunstick")
CLIENTEFFECT_REGISTER_END()
extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);

class C_WeaponStunBaton : public C_BaseHLBludgeonWeapon
{
	DECLARE_CLASS(C_WeaponStunBaton, C_BaseHLBludgeonWeapon);
public:
	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	int DrawModel(int flags)
	{
		//FIXME: This sucks, but I can't easily create temp ents...
		//C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		//if (pPlayer)
			//return 0;

		if (m_Active)
		{
			Vector	vecOrigin;
			QAngle	vecAngles;
			float	color[3];

			color[0] = color[1] = color[2] = random->RandomFloat(0.1f, 0.2f);

			GetAttachment(1, vecOrigin, vecAngles);

			Vector	vForward;
			AngleVectors(vecAngles, &vForward);

			Vector vEnd = vecOrigin;// -vForward * 1.0f;

			IMaterial *pMaterial = materials->FindMaterial("effects/stunstick", NULL, false);

			CMatRenderContextPtr pRenderContext(materials);
			pRenderContext->Bind(pMaterial);
			DrawHalo(pMaterial, vEnd, random->RandomFloat(4.0f, 6.0f), color);

			color[0] = color[1] = color[2] = random->RandomFloat(0.9f, 1.0f);

			DrawHalo(pMaterial, vEnd, random->RandomFloat(2.0f, 3.0f), color);
		}

		return BaseClass::DrawModel(flags);
	}

	// Do part of our effect
	void ClientThink(void)
	{
		C_BasePlayer* pPlayer = C_BasePlayer::GetLocalPlayer();
		if (pPlayer)
			return;

		// Update our effects
		if (m_Active &&
			gpGlobals->frametime != 0.0f &&
			(random->RandomInt(0, 5) == 0))
		{
			//Vector	vecOrigin;
			//QAngle	vecAngles;

			//GetAttachment(1, vecOrigin, vecAngles);

			//Vector	vForward;
			//AngleVectors(vecAngles, &vForward);

			//Vector vEnd = vecOrigin;// -vForward * 1.0f;

			// Inner beams
			BeamInfo_t beamInfo;


			Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			int iAttachment = pOwner->GetViewModel()->LookupAttachment("Sparkrear");
			int iAttachmentEnd = pOwner->GetViewModel()->LookupAttachment("spark_middle");
			pBeamEnt->GetAttachment(iAttachment, vecOrigin2, angAngles2);
			::FormatViewModelAttachment(vecOrigin2, false);


			//beamInfo.m_vecStart = vEnd;
			beamInfo.m_vecStart = vecOrigin2;
			beamInfo.m_pEndEnt = pBeamEnt;
			beamInfo.m_nStartAttachment = iAttachment;

			Vector	offset = RandomVector(-6, 2);

			offset += Vector(2, 2, 2);
			beamInfo.m_vecEnd = vecOrigin2 + offset;

			//beamInfo.m_pStartEnt = cl_entitylist->GetEnt(BEAMENT_ENTITY(entindex()));
			//beamInfo.m_pEndEnt = cl_entitylist->GetEnt(BEAMENT_ENTITY(entindex()));
			//beamInfo.m_nStartAttachment = 1;
			beamInfo.m_nEndAttachment = iAttachmentEnd;

			beamInfo.m_nType = TE_BEAMTESLA;
			beamInfo.m_pszModelName = "sprites/physbeam.vmt";
			beamInfo.m_flHaloScale = 0.0f;
			beamInfo.m_flLife = 0.01f;
			beamInfo.m_flWidth = random->RandomFloat(0.5f, 2.0f);
			beamInfo.m_flEndWidth = 0;
			beamInfo.m_flFadeLength = 0.0f;
			beamInfo.m_flAmplitude = random->RandomFloat(1, 2);
			beamInfo.m_flBrightness = 255.0;
			beamInfo.m_flSpeed = 0.0;
			beamInfo.m_nStartFrame = 0.0;
			beamInfo.m_flFrameRate = 1.0f;
			beamInfo.m_flRed = 255.0f;
			beamInfo.m_flGreen = 255.0f;
			beamInfo.m_flBlue = 255.0f;
			beamInfo.m_nSegments = 8;
			beamInfo.m_bRenderable = true;
			beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE | FBEAM_SHADEOUT);

			beams->CreateBeamPoints(beamInfo);
		}
	}

	void OnDataChanged(DataUpdateType_t updateType)
	{
		BaseClass::OnDataChanged(updateType);
		if (updateType == DATA_UPDATE_CREATED)
		{
			SetNextClientThink(CLIENT_THINK_ALWAYS);
		}
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void StartStunBatonEffect(void)
	{

	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	//-----------------------------------------------------------------------------
	void StopStunBatonEffect(void)
	{
		//TODO: Play shutdown sound
	}

	//-----------------------------------------------------------------------------
	// Purpose: 
	// Output : RenderGroup_t
	//-----------------------------------------------------------------------------
	RenderGroup_t GetRenderGroup(void)
	{
		return RENDER_GROUP_TRANSLUCENT_ENTITY;
	}

private:
	CNetworkVar(bool, m_Active);
};


//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pData - 
//			*pStruct - 
//			*pOut - 
//-----------------------------------------------------------------------------
void RecvProxy_StunBatonActive(const CRecvProxyData *pData, void *pStruct, void *pOut)
{
	bool state = *((bool *)&pData->m_Value.m_Int);

	C_WeaponStunBaton *pWeapon = (C_WeaponStunBaton *)pStruct;

	if (state)
	{
		// Turn on the effect
		pWeapon->StartStunBatonEffect();
	}
	else
	{
		// Turn off the effect
		pWeapon->StopStunBatonEffect();
	}

	*(bool *)pOut = state;
}

STUB_WEAPON_CLASS_IMPLEMENT(weapon_stunbaton, C_WeaponStunBaton);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponStunBaton, DT_WeaponStunBaton, CWeaponStunBaton)
RecvPropInt(RECVINFO(m_Active), 0, RecvProxy_StunBatonActive),
END_RECV_TABLE()





void SparksExplodeCallback(const CEffectData &data)
{
	IClientRenderable *pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		Vector	vecOrigin;
		//TODO: Play startup sound
		
		{
			dlight_t *dl = effects->CL_AllocDlight(data.entindex());	// BriJee OVR : Light our entity
			dl->origin = position;	//effect_origin;
			dl->color.r = 40;
			dl->color.g = 75;
			dl->color.b = 255;
			dl->color.exponent = 5;
			dl->radius = 100.0f * (engine->MapHasHDRLighting() ? 1 : 0.3);
			dl->die = gpGlobals->curtime + 0.9f; //0.2f;
			dl->decay = 512.0f;//0.05f; //512
		}

		BeamInfo_t beamInfo;

		beamInfo.m_vecStart = position;
		beamInfo.m_nStartAttachment = data.m_nAttachmentIndex;
		Vector	offset = RandomVector(-6, 2);

		offset += Vector(4, 4, 4);
		beamInfo.m_vecEnd = position + offset;
		beamInfo.m_nType = TE_BEAMTESLA;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.05f;
		beamInfo.m_flWidth = random->RandomFloat(1.5f, 3.5f);
		beamInfo.m_flEndWidth = 0;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = random->RandomFloat(2, 4);
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 1.0f;
		beamInfo.m_flRed = 179.0f;
		beamInfo.m_flGreen = 0.0f;
		beamInfo.m_flBlue = 255.0f;
		beamInfo.m_nSegments = 15;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE | FBEAM_SHADEOUT);

		beams->CreateBeamPoints(beamInfo);
	}

}
DECLARE_CLIENT_EFFECT("StunsExplode", SparksExplodeCallback);

void SparksEffectCallback(const CEffectData &data)
{
	IClientRenderable *pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		FX_ElectricSpark(position, 1.0f, 1.0f, NULL);
	}

}
DECLARE_CLIENT_EFFECT("StunsSparks", SparksEffectCallback);

void UnderWaterBubblesCallback(const CEffectData &data)
{
	IClientRenderable *pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		UTIL_Bubbles(position, position, 1);
	}

}
DECLARE_CLIENT_EFFECT("UnderWaterBubbles", UnderWaterBubblesCallback);

void MainEffectCallback(const CEffectData &data)
{
	IClientRenderable *pRenderable = data.GetRenderable();
	if (!pRenderable)
		return;

	Vector	position;
	QAngle	angles;

	// If we found the attachment, emit sparks there
	if (pRenderable->GetAttachment(data.m_nAttachmentIndex, position, angles))
	{
		//FX_ElectricSpark(position, 1.0f, 1.0f, NULL);

		Vector	vecOrigin;

		{
			//TODO: Play startup sound
			dlight_t *dl = effects->CL_AllocDlight(data.entindex());	// BriJee OVR : Light our entity
			dl->origin = position;	//effect_origin;
			dl->color.r = 40;
			dl->color.g = 75;
			dl->color.b = 255;
			dl->color.exponent = 5;
			dl->radius = 30.0f * (engine->MapHasHDRLighting() ? 1 : 0.3);
			dl->die = gpGlobals->curtime + 0.1f; //0.2f;
			dl->decay = 512.0f;//0.05f; //0.0f;
		}

		BeamInfo_t beamInfo;

		//beamInfo.m_vecStart = vEnd;
		beamInfo.m_vecStart = position;
		//beamInfo.m_pEndEnt = pRenderable;
		beamInfo.m_nStartAttachment = data.m_nAttachmentIndex;

		Vector	offset = RandomVector(-6, 2);

		offset += Vector(2, 2, 2);
		beamInfo.m_vecEnd = position + offset;

		//beamInfo.m_pStartEnt = cl_entitylist->GetEnt(BEAMENT_ENTITY(entindex()));
		//beamInfo.m_pEndEnt = cl_entitylist->GetEnt(BEAMENT_ENTITY(entindex()));
		//beamInfo.m_nStartAttachment = 1;
		//beamInfo.m_nEndAttachment = iAttachmentEnd;

		beamInfo.m_nType = TE_BEAMTESLA;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.01f;
		beamInfo.m_flWidth = random->RandomFloat(0.5f, 2.0f);
		beamInfo.m_flEndWidth = 0;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = random->RandomFloat(1, 2);
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 1.0f;
		beamInfo.m_flRed = 179.0f;
		beamInfo.m_flGreen = 0.0f;
		beamInfo.m_flBlue = 255.0f;
		beamInfo.m_nSegments = 8;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE | FBEAM_SHADEOUT);

		beams->CreateBeamPoints(beamInfo);
	}

}
DECLARE_CLIENT_EFFECT("StunsEffect", MainEffectCallback);
