
#include "cbase.h"
#include "c_weapon__stubs.h"
#include "ClientEffectPrecacheSystem.h"
#include "c_basehlcombatweapon.h"
#include "basehlcombatweapon_shared.h"
#include "iviewrender_beams.h"
#include "beam_shared.h"
#include "iefx.h"
#include "dlight.h"
#include "c_te_effect_dispatch.h"
#include "input.h"//
#include "engine/ivdebugoverlay.h"


#if 0
//extern bool UTIL_GetWeaponAttachment( C_BaseCombatWeapon *pWeapon, int attachmentID, Vector &absOrigin, QAngle &absAngles );
extern void FormatViewModelAttachment(Vector &vOrigin, bool bInverse);


#include "beamdraw.h"
CLIENTEFFECT_REGISTER_BEGIN(PrecacheEffectEgon)
CLIENTEFFECT_MATERIAL("sprites/physbeam")
CLIENTEFFECT_REGISTER_END()
class C_BeamQuadraticEgon : public CDefaultClientRenderable
{
public:

	C_BeamQuadraticEgon();

	void Update(C_BaseEntity *pOwner);

	virtual const Vector& GetRenderOrigin(void) { return m_worldPosition; }
	virtual const QAngle& GetRenderAngles(void) { return vec3_angle; }
	virtual bool ShouldDraw(void) { return true; }
	virtual bool IsTransparent(void) { return true; }
	virtual bool ShouldReceiveProjectedTextures(int flags) { return false; }
	virtual int	DrawModel(int flags);

	virtual void GetRenderBounds(Vector& mins, Vector& maxs)
	{
		mins.Init(-32, -32, -32);
		maxs.Init(32, 32, 32);
	}

	matrix3x4_t z;

	const matrix3x4_t & RenderableToWorldTransform()
	{
		return z;
	}

	C_BaseEntity *m_pOwner;
	Vector m_targetPosition;
	Vector m_worldPosition;
	int	m_active;
	int	m_glueTouching;
	int	m_viewModelIndex;
};

C_BeamQuadraticEgon::C_BeamQuadraticEgon()
{
	m_pOwner = NULL;
	m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE;
}

void C_BeamQuadraticEgon::Update(C_BaseEntity *pOwner)
{
	m_pOwner = pOwner;
	if (m_active)
	{
		if (m_hRenderHandle == INVALID_CLIENT_RENDER_HANDLE)
		{
			ClientLeafSystem()->AddRenderable(this, RENDER_GROUP_TRANSLUCENT_ENTITY);
		}
		else
		{
			ClientLeafSystem()->RenderableChanged(m_hRenderHandle);
		}
	}
	else if (!m_active && m_hRenderHandle != INVALID_CLIENT_RENDER_HANDLE)
	{
		ClientLeafSystem()->RemoveRenderable(m_hRenderHandle);
		m_hRenderHandle = INVALID_CLIENT_RENDER_HANDLE;
	}
}


float dot = 0.f;
bool positive = false;

#include "view.h"
ConVar oc_test_egon_clientbeam_amplitude("oc_test_egon_clientbeam_amplitude", "1");
int	C_BeamQuadraticEgon::DrawModel(int)
{
	Vector points[4];
	QAngle tmpAngle;

	if (!m_active)
		return 0;

	C_BaseEntity *pEnt = cl_entitylist->GetEnt(m_viewModelIndex);
	if (!pEnt)
		return 0;
	pEnt->GetAttachment(1, points[0], tmpAngle);



	if (dot < 450 && !positive)
		dot++;
	else
	{
		positive = true;
		if (dot > -450 && positive)
			dot--;
		else if (dot <= 450)
			positive = false;
	}


	points[1] = m_targetPosition;// 0.5 * (points[2] + points[0]);// +RandomVector(-35, 35); //m_targetPosition

	points[2] = -m_targetPosition;

	points[3] = m_worldPosition;

	IMaterial *pMat = materials->FindMaterial("sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
	Vector color;
	if (m_glueTouching)
	{
		color.Init(1, 0, 0);
	}
	else
	{
		color.Init(1, 1, 1);
	}

	float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;

	CMatRenderContextPtr pRenderContext(materials);
	pRenderContext->Bind(pMat);

	DrawBeamQuadraticDouble(points[0], points[1], points[2], points[3], 13, color, scrollOffset);

	/*			Vector points[3];
			points[0] = vecOrigin2;
			points[1] = middle + RandomVector(-40.f, 100.f);
			points[2] = tr.endpos;

			IMaterial *pMat = materials->FindMaterial("sprites/physbeam", TEXTURE_GROUP_CLIENT_EFFECTS);
			Vector color;
			{
				color.Init(1, 1, 1);
			}

			float scrollOffset = gpGlobals->curtime - (int)gpGlobals->curtime;

			CMatRenderContextPtr pRenderContext(materials);
			pRenderContext->Bind(pMat);

			DrawBeamQuadratic(points[0], points[1], points[2], 13, color, scrollOffset);*/
	return 1;
}

class C_WeaponEgon : public C_BaseHLCombatWeapon
{
	DECLARE_CLASS(C_WeaponEgon, C_BaseHLCombatWeapon);
public:

	DECLARE_CLIENTCLASS();
	DECLARE_PREDICTABLE();

	C_WeaponEgon();

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

	bool m_Fire;
	Vector			middle;
	Vector			end;
protected:
	CNewParticleEffect	*m_EBeam;
	float m_fNextZoom;
	int m_nZoomLevel;

	int iLaserAttachment;

	C_BeamQuadraticEgon	m_beam;
	//dlight_t						*m_pELight;
};

STUB_WEAPON_CLASS_IMPLEMENT(weapon_egon, C_WeaponEgon);

IMPLEMENT_CLIENTCLASS_DT(C_WeaponEgon, DT_WeaponEgon, CWeaponEgon)
RecvPropBool(RECVINFO(m_Fire)),
RecvPropVector(RECVINFO(middle)),
RecvPropVector(RECVINFO(end)),
END_RECV_TABLE()


C_WeaponEgon::C_WeaponEgon()
{
	iLaserAttachment = -1;

	m_nZoomLevel = 0;
	m_EBeam = NULL;
	//m_pELight = NULL;
}

void C_WeaponEgon::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	/*if (m_pELight)
	{
	m_pELight->die = gpGlobals->curtime;
	m_pELight = NULL;
	}*/
}

void C_WeaponEgon::OnRestore(void)
{
	SetupAttachmentPoints();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_WeaponEgon::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (iLaserAttachment == -1)
	{
		SetupAttachmentPoints();
	}

	m_beam.Update(this);

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

bool C_WeaponEgon::IsCarriedByLocalPlayer(void)
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

//bool Do = true;

static ConVar oc_test_egon_entity_x("oc_test_egon_entity_x", "0.0", FCVAR_ARCHIVE);
static ConVar oc_test_egon_entity_y("oc_test_egon_entity_y", "-1.0", FCVAR_ARCHIVE);
static ConVar oc_test_egon_entity_z("oc_test_egon_entity_z", "-1.01", FCVAR_ARCHIVE);
void C_WeaponEgon::ClientThink(void)
{
	if (iLaserAttachment == -1)
	{
		SetNextClientThink(CLIENT_THINK_NEVER);

		return;
	}

	if ((IsCarriedByLocalPlayer() && !IsEffectActive(EF_NODRAW) && gpGlobals->frametime != 0.0f && m_fNextZoom <= gpGlobals->curtime) && m_Fire)
	{

		Vector	vecOrigin, vecAngles;
		QAngle	angAngles;

		trace_t tr;

		if (//(cvar->FindVar("oc_state_is_running")->GetInt() == 0) && (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0))
		{
			Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Muzzle");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
			::FormatViewModelAttachment(vecOrigin2, false);

			Vector	forward2, right2, Up2;
			AngleVectors(angAngles, &forward2, &right2, &Up2);

			Vector vecOrigin, vecForward, vecRight, vecUp;
			pOwner->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);

			Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

			vecOriginEnd2 = vecForward;
			vecOriginEnd2 += vecForward + forward2 * oc_test_egon_entity_x.GetFloat();
			vecOriginEnd2 += vecRight + right2 * oc_test_egon_entity_y.GetFloat();
			vecOriginEnd2 += vecUp + Up2 * oc_test_egon_entity_z.GetFloat();
			UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)





			dlight_t *el = effects->CL_AllocDlight(LIGHT_INDEX_MUZZLEFLASH);//( index );
			el->origin = tr.endpos;

			el->color.r = 38;
			el->color.g = 151;
			el->color.b = 191;
			el->color.exponent = 5;

			el->radius = random->RandomInt(12, 18);
			el->decay = el->radius / 0.05f;
			el->die = gpGlobals->curtime + 0.07f;

			Vector mid = vecOrigin + (vecOriginEnd2 * 200);
			Vector mid2, mid3;
			float yaw = 4.f;
			VectorYawRotate(mid, yaw, mid2);

			/*Vector fwd, up, rg, res;
			AngleVectors(angAngles, &fwd, &up, &rg);
			res += fwd;
			res += up;
			res += rg;*/
			/*matrix3x4_t forwardToWorld, worldToForward;
			GetAttachment(iLaserAttachment, forwardToWorld);
			MatrixInvert(forwardToWorld, worldToForward);
			VectorIRotate(mid, forwardToWorld, mid2);
			VectorRotate(mid2, forwardToWorld, mid3);*/


			m_beam.m_active = m_Fire;
			m_beam.m_viewModelIndex = pBeamEnt->index;
			m_beam.m_pOwner = pOwner;
			m_beam.m_targetPosition = mid2;//middle;// +RandomVector(-40.f, 100.f);
			m_beam.m_worldPosition = end;
			/*if (m_EBeam == NULL)
			{
				//PrecacheParticleSystem("striderbuster_flechette_attached_lightning6");
				m_EBeam = ParticleProp()->Create("laser_sight", PATTACH_POINT_FOLLOW, iLaserAttachment);
				if (m_EBeam->IsValid())
				{
					m_EBeam->StartEmission();
					m_EBeam->SetOwner(pBeamEnt);
				}
				//m_EBeam = ParticleProp()->Create("weapon_egon_beam", PATTACH_POINT_FOLLOW, iLaserAttachment);
				//m_EBeam = ParticleProp()->Create("controller_fireball", PATTACH_POINT_FOLLOW, iLaserAttachment);
			}
			if (m_EBeam)
			{
				//m_EBeam->RandomVectorInUnitSphere
				//debugoverlay->AddLineOverlay(vecOrigin2, middle, 255, 0, 0, false, 10);
				m_EBeam->SetControlPoint(0, vecOrigin2);
				m_EBeam->SetControlPoint(1, middle + RandomVector(-40.f, 100.f));
				m_EBeam->SetControlPoint(2, tr.endpos);
			}*/
		}
		else if (//(cvar->FindVar("oc_state_is_running")->GetInt() == 1) || (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1))
		{
			//m_EBeam->SetRemoveFlag();

		}

	}
	else
	{
		m_beam.m_active = false;

		/*if (m_EBeam != NULL)
		{
			//m_EBeam->StopEmission();

			//if (m_EBeam->m_bEmissionStopped)
			{
				m_EBeam->SetRemoveFlag();
				m_EBeam = NULL;
			}
		}*/
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the attachment point lookup for the model
//-----------------------------------------------------------------------------
void C_WeaponEgon::SetupAttachmentPoints(void)
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


#endif

#if 0
C_WeaponEgon::C_WeaponEgon()
{
	iLaserAttachment = -1;

	m_nZoomLevel = 0;
	m_EBeam = NULL;
	//m_pELight = NULL;
}

void C_WeaponEgon::UpdateOnRemove()
{
	BaseClass::UpdateOnRemove();

	/*if (m_pELight)
	{
	m_pELight->die = gpGlobals->curtime;
	m_pELight = NULL;
	}*/
}

void C_WeaponEgon::OnRestore(void)
{
	SetupAttachmentPoints();

	BaseClass::OnRestore();
}

//-----------------------------------------------------------------------------
// Purpose: Starts the client-side version thinking
//-----------------------------------------------------------------------------
void C_WeaponEgon::OnDataChanged(DataUpdateType_t updateType)
{
	BaseClass::OnDataChanged(updateType);
	if (iLaserAttachment == -1)
	{
		SetupAttachmentPoints();
	}

	SetNextClientThink(CLIENT_THINK_ALWAYS);
}

bool C_WeaponEgon::IsCarriedByLocalPlayer(void)
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

//bool Do = true;

static ConVar oc_test_egon_entity_x("oc_test_egon_entity_x", "0.0", FCVAR_ARCHIVE);
static ConVar oc_test_egon_entity_y("oc_test_egon_entity_y", "-1.0", FCVAR_ARCHIVE);
static ConVar oc_test_egon_entity_z("oc_test_egon_entity_z", "-1.01", FCVAR_ARCHIVE);
void C_WeaponEgon::ClientThink(void)
{
	if (iLaserAttachment == -1)
	{
		SetNextClientThink(CLIENT_THINK_NEVER);

		return;
	}

	if ((IsCarriedByLocalPlayer() && !IsEffectActive(EF_NODRAW) && gpGlobals->frametime != 0.0f && m_fNextZoom <= gpGlobals->curtime) && m_Fire)
	{

		Vector	vecOrigin, vecAngles;
		QAngle	angAngles;

		trace_t tr;

		if (//(cvar->FindVar("oc_state_is_running")->GetInt() == 0) &&  (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 0))
		{
			Vector vecOrigin2, vecOriginEnd2;
			QAngle angAngles2;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Muzzle");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin2, angAngles);
			::FormatViewModelAttachment(vecOrigin2, false);

			Vector	forward2, right2, Up2;
			AngleVectors(angAngles, &forward2, &right2, &Up2);

			Vector vecOrigin, vecForward, vecRight, vecUp;
			pOwner->EyePositionAndVectors(&vecOrigin, &vecForward, &vecRight, &vecUp);

			Vector vecStart = vecOrigin + (vecRight * 4) - (vecUp * 4);

			vecOriginEnd2 = vecForward;
			vecOriginEnd2 += vecForward + forward2 * oc_test_egon_entity_x.GetFloat();
			vecOriginEnd2 += vecRight + right2 * oc_test_egon_entity_y.GetFloat();
			vecOriginEnd2 += vecUp + Up2 * oc_test_egon_entity_z.GetFloat();
			UTIL_TraceLine(vecOrigin, vecOrigin + (vecForward * MAX_TRACE_LENGTH), MASK_SHOT, pOwner, COLLISION_GROUP_NONE, &tr);//vecOrigin + (vecOriginEnd2 * MAX_TRACE_LENGTH)



			dlight_t *el = effects->CL_AllocDlight(LIGHT_INDEX_MUZZLEFLASH);//( index );
			el->origin = tr.endpos;

			el->color.r = 38;
			el->color.g = 151;
			el->color.b = 191;
			el->color.exponent = 5;

			el->radius = random->RandomInt(12, 18);
			el->decay = el->radius / 0.05f;
			el->die = gpGlobals->curtime + 0.07f;

			if (m_EBeam == NULL)
			{
				//PrecacheParticleSystem("striderbuster_flechette_attached_lightning6");
				m_EBeam = ParticleProp()->Create("weapon_egon_beam", PATTACH_POINT_FOLLOW, iLaserAttachment);
				m_EBeam->StartEmission();
				m_EBeam->SetOwner(pBeamEnt);
			}
			if (m_EBeam)
			{
				m_EBeam->SetControlPoint(0, vecOrigin2);
				m_EBeam->SetControlPoint(1, tr.endpos);
			}
		}
		else if (//(cvar->FindVar("oc_state_is_running")->GetInt() == 1) || (cvar->FindVar("oc_state_near_wall_standing")->GetInt() == 1))
		{
			Vector vecOrigin3, vecAngles3;
			QAngle angAngles3;
			C_BasePlayer *pOwner = ToBasePlayer(GetOwner());
			C_BaseViewModel *pBeamEnt = pOwner->GetViewModel();
			iLaserAttachment = pOwner->GetViewModel()->LookupAttachment("Laser");
			pBeamEnt->GetAttachment(iLaserAttachment, vecOrigin3, angAngles3);
			::FormatViewModelAttachment(vecOrigin3, false);


			if (m_EBeam == NULL)
			{
				//PrecacheParticleSystem("striderbuster_flechette_attached_lightning6");
				m_EBeam = ParticleProp()->Create("weapon_egon_beam", PATTACH_POINT_FOLLOW, iLaserAttachment);//striderbuster_flechette_attached_lightning6//lc_beam_red_tint
				m_EBeam->StartEmission();
				//m_EBeam->SetOwner(pBeamEnt);
			}
			if (m_EBeam)
			{
				m_EBeam->SetControlPoint(0, vecOrigin3);
				m_EBeam->SetControlPoint(1, tr.endpos);
			}
		}

	}
	else
	{
		if (m_EBeam)
		{
			m_EBeam->StopEmission();
			
			if (m_EBeam->m_bEmissionStopped)
				m_EBeam = NULL;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Sets up the attachment point lookup for the model
//-----------------------------------------------------------------------------
void C_WeaponEgon::SetupAttachmentPoints(void)
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

#endif

void EgonEffectCallback(const CEffectData &data)
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
		/*dlight_t *dl = effects->CL_AllocDlight(1);
		dl->origin = position;
		dl->color.r = 40;
		dl->color.g = 75;
		dl->color.b = 255;
		dl->color.exponent = 5;
		dl->radius = 30.0f;
		dl->die = gpGlobals->curtime + 0.1f;
		dl->decay = 512.0f;*/

		BeamInfo_t beamInfo;

		beamInfo.m_vecStart = position;
		beamInfo.m_nStartAttachment = data.m_nAttachmentIndex;

		Vector	offset = RandomVector(-16, 16);

		offset += Vector(random->RandomFloat(-32, 32), random->RandomFloat(-32, 32), random->RandomFloat(-32, 32));
		beamInfo.m_vecEnd = position + offset;

		beamInfo.m_nType = TE_BEAMTESLA;
		beamInfo.m_pszModelName = "sprites/physbeam.vmt";
		beamInfo.m_flHaloScale = 0.0f;
		beamInfo.m_flLife = 0.04f;
		beamInfo.m_flWidth = random->RandomFloat(1.0f, 2.5f);
		beamInfo.m_flEndWidth = 0;
		beamInfo.m_flFadeLength = 0.0f;
		beamInfo.m_flAmplitude = random->RandomFloat(6, 9);
		beamInfo.m_flBrightness = 255.0;
		beamInfo.m_flSpeed = 0.0;
		beamInfo.m_nStartFrame = 0.0;
		beamInfo.m_flFrameRate = 1.0f;
		beamInfo.m_flRed = 112;
		beamInfo.m_flGreen = 39;
		beamInfo.m_flBlue = 195;
		beamInfo.m_nSegments = 25;
		beamInfo.m_bRenderable = true;
		beamInfo.m_nFlags = (FBEAM_ONLYNOISEONCE | FBEAM_SHADEOUT);

		beams->CreateBeamPoints(beamInfo);
	}

}
DECLARE_CLIENT_EFFECT("EgonLightning", EgonEffectCallback);