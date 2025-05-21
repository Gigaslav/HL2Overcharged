
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


#include "dlight.h"
#include "r_efx.h"
#include "c_te_effect_dispatch.h"

//extern bool UTIL_GetWeaponAttachment( C_BaseCombatWeapon *pWeapon, int attachmentID, Vector &absOrigin, QAngle &absAngles );
extern void FormatViewModelAttachment( Vector &vOrigin, bool bInverse );

class C_WeaponIonRifle : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponIonRifle, C_BaseHLCombatWeapon);
public:

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_WeaponIonRifle();

	virtual bool		VisibleInWeaponSelection( void ) { return true; }
	virtual bool		CanBeSelected( void ) { return true; }
	virtual bool		HasAnyAmmo( void ) { return true; }
	virtual bool		HasAmmo( void ) {  return true; }

	void				OnRestore( void );
	void				UpdateOnRemove( void );

	bool				IsCarriedByLocalPlayer();

	virtual void ClientThink( void );
	virtual void OnDataChanged( DataUpdateType_t updateType );

	virtual bool IsWeaponCamera() { return (m_nZoomLevel == 0); }

	void SetupAttachmentPoints();

	CNewParticleEffect *pEffect;
	EHANDLE				m_hEntAttached;

	bool EnableLaser;

protected:

	float m_fNextZoom;
	int m_nZoomLevel;

	int iLaserAttachment;

};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_ionrifle, C_WeaponIonRifle);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponIonRifle, DT_WeaponIonRifle, CWeaponIonRifle)
RecvPropBool(RECVINFO(EnableLaser)),
END_RECV_TABLE()

C_WeaponIonRifle::C_WeaponIonRifle()
{
	iLaserAttachment = -1;

	m_nZoomLevel = 0;

	//m_pELight = NULL;
}

void C_WeaponIonRifle::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();
}

void C_WeaponIonRifle::OnRestore(void)
{
	SetupAttachmentPoints();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_WeaponIonRifle::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged( updateType );
	if ( iLaserAttachment == -1 )
	{
		SetupAttachmentPoints();
	}

	SetNextClientThink( CLIENT_THINK_ALWAYS );
}

bool C_WeaponIonRifle::IsCarriedByLocalPlayer(void)
{
	CBaseViewModel *vm = NULL;
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	if ( pOwner )
	{
		if ( pOwner->GetActiveWeapon() != this )
			return false;

		vm = pOwner->GetViewModel( m_nViewModelIndex );
		if ( vm )
			return ( !vm->IsEffectActive(EF_NODRAW) );
	}

	return false;
}


static ConVar oc_weapon_ionrifle_laser_pos_x("oc_weapon_ionrifle_laser_pos_x", "0.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_ionrifle_laser_pos_y("oc_weapon_ionrifle_laser_pos_y", "-1.0", FCVAR_ARCHIVE);
static ConVar oc_weapon_ionrifle_laser_pos_z("oc_weapon_ionrifle_laser_pos_z", "-1.01", FCVAR_ARCHIVE);

static ConVar oc_weapon_ionrifle_laser_ang_x("oc_weapon_ionrifle_laser_ang_x", "-0", FCVAR_ARCHIVE);
static ConVar oc_weapon_ionrifle_laser_ang_y("oc_weapon_ionrifle_laser_ang_y", "-0", FCVAR_ARCHIVE);
static ConVar oc_weapon_ionrifle_laser_ang_z("oc_weapon_ionrifle_laser_ang_z", "-0", FCVAR_ARCHIVE);

void C_WeaponIonRifle::ClientThink(void)
{

	if (iLaserAttachment == -1)
	{
		SetNextClientThink( CLIENT_THINK_NEVER );

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
			Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
			::FormatViewModelAttachment(vecOrigin2, false);

			Vector	forward2, right2, Up2;
			angAngles.x += oc_weapon_ionrifle_laser_ang_x.GetFloat();
			angAngles.y += oc_weapon_ionrifle_laser_ang_y.GetFloat();
			angAngles.z += oc_weapon_ionrifle_laser_ang_z.GetFloat();
			AngleVectors(angAngles, &forward2, &right2, &Up2);

			//Straith from muzzle
			/*Vector vecOrigin3 = forward2 + right2 + Up2 * MAX_TRACE_LENGTH;
			UTIL_TraceLine(vecOrigin2, vecOrigin2 + vecOrigin3, MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
			beamInfo.m_vecEnd = vecOrigin2;
			beamInfo.m_pEndEnt = pBeamEnt;
			beamInfo.m_nEndAttachment = iLaserAttachment;
			beamInfo.m_vecStart = tr.endpos;
			beamInfo.m_pStartEnt = NULL;
			beamInfo.m_nStartAttachment = -1;*/

			Vector vecOrigin, vecForward, vecRight, vecUp;
			pOwner->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);

			Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

			/*beamInfo.m_vecEnd = vecOrigin2;
			beamInfo.m_pEndEnt = pBeamEnt;
			beamInfo.m_nEndAttachment = iLaserAttachment;
			//beamInfo.m_vecCenter += RandomVector(-30.f,30.f);
			vecOriginEnd2 = vecForward + vecRight + vecUp;//vecForward
			vecOriginEnd2 += forward2 * oc_weapon_ionrifle_laser_x.GetFloat();
			vecOriginEnd2 += right2 * oc_weapon_ionrifle_laser_y.GetFloat();
			vecOriginEnd2 += Up2 * oc_weapon_ionrifle_laser_z.GetFloat();

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
				vecOriginEnd2 += vecForward + forward2 * oc_weapon_ionrifle_laser_pos_x.GetFloat();
				vecOriginEnd2 += vecRight + right2 * oc_weapon_ionrifle_laser_pos_y.GetFloat();
				vecOriginEnd2 += vecUp + Up2 * oc_weapon_ionrifle_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
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
				vecOriginEnd2 += vecForward + forward2 * oc_weapon_ionrifle_laser_pos_x.GetFloat();
				vecOriginEnd2 += vecRight + right2 * oc_weapon_ionrifle_laser_pos_y.GetFloat();
				vecOriginEnd2 += vecUp + Up2 * oc_weapon_ionrifle_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
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
				vecOriginEnd2 += forward2 * oc_weapon_ionrifle_laser_pos_x.GetFloat();
				vecOriginEnd2 += right2 * oc_weapon_ionrifle_laser_pos_y.GetFloat();
				vecOriginEnd2 += Up2 * oc_weapon_ionrifle_laser_pos_z.GetFloat();
				UTIL_TraceLine(vecOrigin, vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);
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


			/*if (pEffect == NULL)
			{
				pEffect = ParticleProp()->Create("ion_laser_dot", PATTACH_POINT_FOLLOW, iLaserAttachment);
				pEffect->SetOwner(pBeamEnt);
				pEffect->StartEmission();

				ParticleProp()->AddControlPoint(pEffect, 0, pBeamEnt, PATTACH_POINT_FOLLOW);
				//ParticleProp()->AddControlPoint(pEffect, 1, pBeamEnt, PATTACH_POINT_FOLLOW);
			}
			if (pEffect != NULL)
			{
				pEffect->SetControlPoint(0, vecOrigin);

				//pEffect->SetControlPoint(1, vecOrigin);
			}*/

			//DispatchParticleEffect("ion_laser_dot", PATTACH_POINT_FOLLOW, pBeamEnt, "laser", false);
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

			/*if (pEffect != NULL)
			{
				pEffect->StopEmission();
				pEffect = NULL;
			}*/
			//StopParticleEffects(pBeamEnt);
		}


		beamInfo.m_pszModelName = "effects/bluelaser1.vmt";

		if ( !(tr.surface.flags & SURF_SKY) )
		{
			beamInfo.m_pszHaloName = "sprites/light_glow03.vmt";
			beamInfo.m_flHaloScale = 4.0f;
		}

		beamInfo.m_flLife = 0.01f;
		beamInfo.m_flWidth = 1.0f; //random->RandomFloat( 1.0f, 2.0f );
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
		
		beams->CreateBeamEntPoint( beamInfo );
	}
	else
	{
		//StopParticleEffects(this);
		/*if (pEffect != NULL)
		{
			pEffect->StopEmission();
			pEffect = NULL;
		}*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the attachment point lookup for the model
//-----------------------------------------------------------------------------
void C_WeaponIonRifle::SetupAttachmentPoints(void)
{
	CBasePlayer *pOwner = ToBasePlayer( GetOwner() );
	
	if ( pOwner != NULL && pOwner->GetViewModel() != NULL )
	{
			// Setup the center beam point
		iLaserAttachment = pOwner->GetViewModel()->LookupAttachment( "Laser" );
	}
	else
	{
		iLaserAttachment = -1;
	}
}

void IonMuzzleLightEffectCallback(const CEffectData &data)
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
		dlight_t *dl = effects->CL_AllocDlight(2);	// BriJee OVR : Light our entity
		dl->origin = position;	//effect_origin;
		dl->color.r = 40;
		dl->color.g = 135;
		dl->color.b = 255;
		dl->color.exponent = 5;
		dl->radius = 50.0f;
		dl->die = gpGlobals->curtime + 0.9f; //0.2f;
		dl->decay = 512.0f;//0.05f; //512
	}

}
DECLARE_CLIENT_EFFECT("IonMuzzleLight", IonMuzzleLightEffectCallback);