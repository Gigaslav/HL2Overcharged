//========= Copyright Valve Corporation, All rights reserved. ============//
//
// Purpose: Implements C_Corpse
//
// $NoKeywords: $
//=============================================================================//
#include "cbase.h"
#include "c_player_model.h"

#include "cbase.h"
#include "vcollide_parse.h"

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

//NOTENOTE: This is not yet coupled with the server-side implementation of CGib
//			This is only a client-side version of gibs at the moment

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
C_Player_Model::~C_Player_Model(void)
{
	VPhysicsDestroyObject();
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszModelName - 
//			vecOrigin - 
//			vecForceDir - 
//			vecAngularImp - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
C_Player_Model *C_Player_Model::CreateClientsideGib(const Vector &vecOrigin)
{
	C_Player_Model *pPlrModel = new C_Player_Model;

	if (pPlrModel == NULL)
		return NULL;

	if (pPlrModel->InitializeGib(vecOrigin) == false)
		return NULL;

	return pPlrModel;
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pszModelName - 
//			vecOrigin - 
//			vecForceDir - 
//			vecAngularImp - 
// Output : Returns true on success, false on failure.
//-----------------------------------------------------------------------------
bool C_Player_Model::InitializeGib(const Vector &vecOrigin)
{
	PrecacheModel(cvar->FindVar("oc_playermodel_suit")->GetString());

	if (InitializeAsClientEntity(cvar->FindVar("oc_playermodel_suit")->GetString(), RENDER_GROUP_OPAQUE_ENTITY) == false)
	{
		Release();
		return false;
	}

	SetModel(cvar->FindVar("oc_playermodel_suit")->GetString());

	SetAbsOrigin(vecOrigin);
	SetCollisionGroup(COLLISION_GROUP_NONE);

	solid_t tmpSolid;
	PhysModelParseSolid(tmpSolid, this, GetModelIndex());

	m_pPhysicsObject = VPhysicsInitNormal(SOLID_VPHYSICS, 0, false, &tmpSolid);

	if (m_pPhysicsObject)
	{

	}
	else
	{

	}

	SetNextClientThink(gpGlobals->curtime + 0.1f);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void C_Player_Model::ClientThink(void)
{
	/*SetRenderMode(kRenderTransAlpha);
	m_nRenderFX = kRenderFxFadeFast;

	if (m_clrRender->a == 0)
	{

		Release();
		return;
		}*/	
	C_BasePlayer *pPlayer = C_BasePlayer::GetLocalPlayer();
	if (pPlayer)
	{
		matrix3x4_t& bone = GetBoneForWrite(2);
		
		Vector vBonePos;
		Vector orig = pPlayer->EyePosition();
		MatrixGetTranslation(bone, vBonePos);
		vBonePos = orig;
		MatrixSetTranslation(vBonePos, bone);
		ApplyBoneMatrixTransform(bone);
		/*bone.m_flMatVal[0][0] = pPlayer->EyeAngles().x;
		bone.m_flMatVal[1][0] = pPlayer->EyeAngles().x;
		bone.m_flMatVal[2][0] = pPlayer->EyeAngles().x;*/
	}
	SetNextClientThink(gpGlobals->curtime + 1.0f);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_Player_Model::StartTouch(C_BaseEntity *pOther)
{
	// Limit the amount of times we can bounce
	if (m_flTouchDelta < gpGlobals->curtime)
	{
		HitSurface(pOther);
		m_flTouchDelta = gpGlobals->curtime + 0.1f;
	}

	BaseClass::StartTouch(pOther);
}

//-----------------------------------------------------------------------------
// Purpose: 
// Input  : *pOther - 
//-----------------------------------------------------------------------------
void C_Player_Model::HitSurface(C_BaseEntity *pOther)
{
	//TODO: Implement splatter or effects in child versions
}
