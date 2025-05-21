
#include "cbase.h"
#include "c_weapon__stubs.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "iefx.h"
#include "dlight.h"

#include "input.h"//
#include "engine/ivdebugoverlay.h"



//extern bool UTIL_GetWeaponAttachment( C_BaseCombatWeapon *pWeapon, int attachmentID, Vector &absOrigin, QAngle &absAngles );
extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);

class C_WeaponCombineSniper : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponCombineSniper, C_BaseHLCombatWeapon);
public:

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_WeaponCombineSniper();

	virtual bool		VisibleInWeaponSelection(void) { return true; }
	virtual bool		CanBeSelected(void) { return true; }
	virtual bool		HasAnyAmmo(void) { return true; }
	virtual bool		HasAmmo(void) { return true; }

	void				OnRestore(void);
	void				UpdateOnRemove(void);

	bool				IsCarriedByLocalPlayer();

	virtual void ClientThink(void);
	virtual void OnDataChanged(DataUpdateType_t updateType);

	virtual bool IsWeaponCamera() { return (m_nZoomLevel == 0); }

	void SetupAttachmentPoints();

	CNewParticleEffect *pEffect;
	EHANDLE				m_hEntAttached;

	bool EnableLaser;

protected:

	float m_fNextZoom;
	float m_nZoomLevel;

	int iLaserAttachment;

	//dlight_t						*m_pELight;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_combinesniper, C_WeaponCombineSniper);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponCombineSniper, DT_WeaponCombineSniper, CWeaponCombineSniper)
RecvPropBool(RECVINFO(EnableLaser)),
RecvPropFloat(RECVINFO(m_fNextZoom)),
RecvPropInt(RECVINFO(m_nZoomLevel)),

END_RECV_TABLE()

C_WeaponCombineSniper::C_WeaponCombineSniper()
{
	iLaserAttachment = -1;

	m_nZoomLevel = 0;

	//m_pELight = NULL;
}

void C_WeaponCombineSniper::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	/*if (m_pELight)
	{
	m_pELight->die = gpGlobals->curtime;
	m_pELight = NULL;
	}*/
}

void C_WeaponCombineSniper::OnRestore(void)
{
	SetupAttachmentPoints();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_WeaponCombineSniper::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (iLaserAttachment == -1)
	{
		SetupAttachmentPoints();
	}

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

bool C_WeaponCombineSniper::IsCarriedByLocalPlayer(void)
{
	CBaseViewModel *vm = NULL;
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());
	if (pOwner)
	{
		if (pOwner->GetActiveWeapon() != this)
			return false;

		vm = pOwner->GetViewModel(m_nViewModelIndex);
		if (vm)
			return (!vm->IsEffectActive(EF_NODRAW));
	}

	return false;
}

extern ConVar oc_weapon_combine_sniper_laser_pos_x("oc_weapon_combine_sniper_laser_pos_x", "0.0", FCVAR_ARCHIVE);
extern ConVar oc_weapon_combine_sniper_laser_pos_y("oc_weapon_combine_sniper_laser_pos_y", "0.0", FCVAR_ARCHIVE);
extern ConVar oc_weapon_combine_sniper_laser_pos_z("oc_weapon_combine_sniper_laser_pos_z", "0.0", FCVAR_ARCHIVE);

extern ConVar oc_weapon_combine_sniper_laser_ang_x("oc_weapon_combine_sniper_laser_ang_x", "0", FCVAR_ARCHIVE);
extern ConVar oc_weapon_combine_sniper_laser_ang_y("oc_weapon_combine_sniper_laser_ang_y", "0", FCVAR_ARCHIVE);
extern ConVar oc_weapon_combine_sniper_laser_ang_z("oc_weapon_combine_sniper_laser_ang_z", "0", FCVAR_ARCHIVE);
void C_WeaponCombineSniper::ClientThink(void)
{

	if (iLaserAttachment == -1)
	{
		SetNextClientThink(CLIENT_THINK_NEVER);

		/*if (m_pELight)
		{
		m_pELight->die = gpGlobals->curtime;
		m_pELight = NULL;
		}*/
		return;
	}
	
	if ((IsCarriedByLocalPlayer() && !IsEffectActive(EF_NODRAW) && gpGlobals->frametime != 0.0f && m_fNextZoom <= gpGlobals->curtime) && EnableLaser)
	{


		Vector	vecOrigin, vecAngles;
		QAngle	angAngles;

		// Inner beams
		BeamInfo_t beamInfo;

		trace_t tr;

		/*if (cvar->FindVar("oc_state_InReload")->GetInt() == 1)// || (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1))//(m_nZoomLevel == 1)
		{
		C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
		C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
		//C_BaseViewModel *pBeamEnt2 = pOwner->GetViewModel();
		//C_BaseEntity *pEntity = m_hEntAttached;

		iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
		//pEntity->FollowEntity(pBeamEnt);
		pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin, angAngles);

		//UTIL_GetWeaponAttachment( this, iLaserAttachment, vecOrigin, angAngles );
		::FormatViewModelAttachment( vecOrigin, false );

		beamInfo.m_vecEnd = vec3_origin;
		beamInfo.m_pEndEnt = pBeamEnt;
		beamInfo.m_nEndAttachment = iLaserAttachment;


		Vector forward, right, up;
		pOwner->EyeVectors(&forward, &right, &up);


		AngleVectors(angAngles, &vecAngles);
		UTIL_TraceLine(vecOrigin, vecOrigin + (vecAngles * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
		//debugoverlay->AddLineOverlay(vecOrigin, pOwner->EyePosition() + (forward * MAX_TRACE_LENGTH), 255, 0, 0, true, 0.2f);

		beamInfo.m_vecStart = tr.endpos;
		beamInfo.m_pStartEnt = NULL;
		beamInfo.m_nStartAttachment = -1;


		Vector Muzzle;
		Muzzle.x = cvar->FindVar("oc_muzzle_vector_x")->GetFloat();
		Muzzle.y = cvar->FindVar("oc_muzzle_vector_y")->GetFloat();
		Muzzle.z = cvar->FindVar("oc_muzzle_vector_z")->GetFloat();


		pBeamEnt->SetAbsOrigin(vecOrigin);*/


		//pBeamEnt2->SetAbsOrigin(vecOrigin);

		/*vecOrigin += forward * oc_test_entity_x.GetFloat();
		vecOrigin += right * oc_test_entity_y.GetFloat();
		vecOrigin += up * oc_test_entity_z.GetFloat();*/

		/*vecOrigin.x += oc_test_entity_x.GetFloat();
		vecOrigin.y += oc_test_entity_y.GetFloat();
		vecOrigin.z += oc_test_entity_z.GetFloat();*/

		//if (Do)
		/*{
		pEffect = ParticleProp()->Create("sniper_laser", PATTACH_POINT_FOLLOW);

		ParticleProp()->AddControlPoint(pEffect, 0, pBeamEnt2, PATTACH_ABSORIGIN_FOLLOW, "Laser", vecOrigin);
		//ParticleProp()->AddControlPoint(pEffect, 1, pBeamEnt, PATTACH_POINT, "Laser", tr.endpos);
		//pEffect->SetControlPoint(1, vecOrigin + vecOrigin * 100);// +(vecAngles * MAX_TRACE_LENGTH));
		//pEffect->SetControlPointEntity(1, pBeamEnt);
		pEffect->SetControlPoint(1, tr.endpos);
		//pEffect->SetControlPoint(0, vecOrigin);// +(vecAngles * MAX_TRACE_LENGTH));
		//pEffect->SetControlPointEntity(0, pBeamEnt);
		pEffect->SetControlPointEntity(1, pBeamEnt);


		}*/

		//}
		if ((cvar->FindVar("oc_state_is_running")->GetInt() == 0) && (cvar->FindVar("oc_state_InReload")->GetInt() == 0) && (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0))
		{
			/*Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
			::FormatViewModelAttachment(vecOrigin2, false);

			Vector	forward2, right2, Up2;
			AngleVectors(angAngles, &forward2, &right2, &Up2);
			//			CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
			Vector vecOrigin, vecForward, vecRight, vecUp;
			pOwner->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);

			Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

			beamInfo.m_vecEnd = vecOrigin2;
			beamInfo.m_pEndEnt = pBeamEnt;
			beamInfo.m_nEndAttachment = iLaserAttachment;

			vecOriginEnd2 = vecForward;
			vecOriginEnd2 += vecForward + forward2 * oc_test_entity_x.GetFloat();
			vecOriginEnd2 += vecRight + right2 * oc_test_entity_y.GetFloat();
			vecOriginEnd2 += vecUp + Up2 * oc_test_entity_z.GetFloat();
			UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)

			beamInfo.m_vecStart = tr.endpos;
			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_nStartAttachment = -1;*/

			Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
			::FormatViewModelAttachment(vecOrigin2, false);

			Vector	forward2, right2, Up2;
			angAngles.x += oc_weapon_combine_sniper_laser_ang_x.GetFloat();
			angAngles.y += oc_weapon_combine_sniper_laser_ang_y.GetFloat();
			angAngles.z += oc_weapon_combine_sniper_laser_ang_z.GetFloat();
			AngleVectors(angAngles, &forward2, &right2, &Up2);

			Vector vecOrigin, vecForward, vecRight, vecUp;
			pOwner->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);

			Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

			/*beamInfo.m_vecEnd = vecOrigin2;
			beamInfo.m_pEndEnt = pBeamEnt;
			beamInfo.m_nEndAttachment = iLaserAttachment;
			//beamInfo.m_vecCenter += RandomVector(-30.f,30.f);
			vecOriginEnd2 = vecForward + vecRight + vecUp;//vecForward
			vecOriginEnd2 += forward2 * oc_test_sniper_entity_x.GetFloat();
			vecOriginEnd2 += right2 * oc_test_sniper_entity_y.GetFloat();
			vecOriginEnd2 += Up2 * oc_test_sniper_entity_z.GetFloat();

			UTIL_TraceLine(vecOrigin, vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)

			beamInfo.m_vecStart = tr.endpos;
			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_nStartAttachment = -1;*/


			if (cvar->FindVar("oc_weapon_free_aim")->GetInt())
			{
				beamInfo.m_vecEnd = vecOrigin2;
				beamInfo.m_pEndEnt = pBeamEnt;
				beamInfo.m_nEndAttachment = iLaserAttachment;
				vecOriginEnd2 = vecForward;
				vecOriginEnd2 += vecForward + forward2 * oc_weapon_combine_sniper_laser_pos_x.GetFloat();
				vecOriginEnd2 += vecRight + right2 * oc_weapon_combine_sniper_laser_pos_y.GetFloat();
				vecOriginEnd2 += vecUp + Up2 * oc_weapon_combine_sniper_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)
				beamInfo.m_vecStart = tr.endpos;
				beamInfo.m_pStartEnt = NULL;
				beamInfo.m_nStartAttachment = -1;
			}
			else if (cvar->FindVar("oc_state_IRsight_on")->GetInt() || cvar->FindVar("oc_state_InSecondFire")->GetInt())
			{
				beamInfo.m_vecEnd = vecOrigin2;
				beamInfo.m_pEndEnt = pBeamEnt;
				beamInfo.m_nEndAttachment = iLaserAttachment;
				vecOriginEnd2 = vecForward;
				vecOriginEnd2 += vecForward + forward2 * oc_weapon_combine_sniper_laser_pos_x.GetFloat();
				vecOriginEnd2 += vecRight + right2 * oc_weapon_combine_sniper_laser_pos_y.GetFloat();
				vecOriginEnd2 += vecUp + Up2 * oc_weapon_combine_sniper_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)
				beamInfo.m_vecStart = tr.endpos;
				beamInfo.m_pStartEnt = NULL;
				beamInfo.m_nStartAttachment = -1;
			}
			else
			{
				beamInfo.m_vecEnd = vecOrigin2;
				beamInfo.m_pEndEnt = pBeamEnt;
				beamInfo.m_nEndAttachment = iLaserAttachment;
				vecOriginEnd2 = vecForward + vecRight + vecUp;
				vecOriginEnd2 += forward2 * oc_weapon_combine_sniper_laser_pos_x.GetFloat();
				vecOriginEnd2 += right2 * oc_weapon_combine_sniper_laser_pos_y.GetFloat();
				vecOriginEnd2 += Up2 * oc_weapon_combine_sniper_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)
				beamInfo.m_vecStart = tr.endpos;
				beamInfo.m_pStartEnt = NULL;
				beamInfo.m_nStartAttachment = -1;
			}


			dlight_t *el = effects->CL_AllocDlight(LIGHT_INDEX_MUZZLEFLASH);//( index );
			el->origin = tr.endpos;

			el->color.r = 38;
			el->color.g = 151;
			el->color.b = 191;
			el->color.exponent = 5;

			el->radius = random->RandomInt(12, 18);
			el->decay = el->radius / 0.05f;
			el->die = gpGlobals->curtime + 0.07f;
			/*if (m_pELight)
			{
			m_pELight->die = gpGlobals->curtime;
			m_pELight = NULL;
			}*/
		}
		else if ((cvar->FindVar("oc_state_is_running")->GetInt() == 1) || (cvar->FindVar("oc_state_InReload")->GetInt() == 1) || (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1))
		{
			Vector vecOrigin3, vecAngles3;
			QAngle angAngles3;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin3, angAngles3);
			::FormatViewModelAttachment(vecOrigin3, false);


			beamInfo.m_vecEnd = vecOrigin3;
			beamInfo.m_pEndEnt = NULL;
			beamInfo.m_nEndAttachment = -1;
			AngleVectors(angAngles3, &vecAngles3);
			UTIL_TraceLine(vecOrigin3, vecOrigin3 + (vecAngles3 * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);

			beamInfo.m_vecStart = vecOrigin3;
			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_nStartAttachment = -1;
		}


		beamInfo.m_pszModelName = "effects/bluelaser1.vmt";

		if (!(tr.surface.flags & SURF_SKY))
		{
			beamInfo.m_pszHaloName = "sprites/light_glow03.vmt";
			beamInfo.m_flHaloScale = 4.0f;
		}

		beamInfo.m_flLife = 0.01f;
		if (cvar->FindVar("oc_state_InSecondFire")->GetInt())
			beamInfo.m_flWidth = m_bZoomLevel*0.01f; //random->RandomFloat( 1.0f, 2.0f );
		else
			beamInfo.m_flWidth = 0.8f;
		//DevMsg("beamInfo.m_flWidth: %.2f \n", beamInfo.m_flWidth);
		beamInfo.m_flEndWidth = 0;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = 0.0f; //random->RandomFloat( 16, 32 );
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 1.0f;
		beamInfo.m_flRed = 0.0f;;
		beamInfo.m_flGreen = 0.0f;
		beamInfo.m_flBlue = 255.0f;
		beamInfo.m_nSegments = 0;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = 0;

		beams->CreateBeamEntPoint(beamInfo);
	}

}

//-----------------------------------------------------------------------------
// Purpose: Sets up the attachment point lookup for the model
//-----------------------------------------------------------------------------
void C_WeaponCombineSniper::SetupAttachmentPoints(void)
{
	CBasePlayer *pOwner = ToBasePlayer(GetOwner());

	if (pOwner != NULL && pOwner->GetViewModel() != NULL)
	{
		// Setup the center beam point
		iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
	}
	else
	{
		iLaserAttachment = -1;
	}
}