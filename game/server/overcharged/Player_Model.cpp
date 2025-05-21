#include "cbase.h"
#include "hltvdirector.h"
#include "OverCharged/Player_Model.h"
#include "datacache/imdlcache.h"
#include "tier0/icommandline.h"
#include "tier0/memdbgon.h"

ConVar oc_player_upperhalf_origin_x("oc_player_upperhalf_origin_x", "-10");
ConVar oc_player_upperhalf_origin_y("oc_player_upperhalf_origin_y", "3");
ConVar oc_player_upperhalf_origin_z("oc_player_upperhalf_origin_z", "-55");



ConVar oc_player_apply_offsets("oc_player_apply_offsets", "0");

LINK_ENTITY_TO_CLASS(player_model, CPlayerModel);
PRECACHE_REGISTER(player_model);

BEGIN_DATADESC(CPlayerModel)
	DEFINE_FIELD(inCrouch, FIELD_BOOLEAN),
	DEFINE_FIELD(posZ, FIELD_FLOAT),
END_DATADESC()

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModel::Spawn(void)
{
	Precache();

	//PrecacheModel("models/weapons/c17/v_smg1.mdl");
	//SetModel("models/weapons/c17/v_smg1.mdl");
	SetModel(cvar->FindVar("oc_playermodel_suit")->GetString());

	inCrouch = false;

	//CHL2_Player *pPlayer = (CHL2_Player *)GetOwnerEntity();
	//newOrigin = pPlayer->GetLocalOrigin();
	int bg_rh = FindBodygroupByName("right_hand");
	int bg_lh = FindBodygroupByName("left_hand");
	SetBodygroup(bg_rh, 0);
	SetBodygroup(bg_lh, 0);
	int bg_rl = FindBodygroupByName("right_leg");
	int bg_ll = FindBodygroupByName("left_leg");
	int bg_t = FindBodygroupByName("torso");
	int bg_h = FindBodygroupByName("head");
	SetBodygroup(bg_rl, 1);
	SetBodygroup(bg_ll, 1);
	SetBodygroup(bg_t, 1);
	SetBodygroup(bg_h, 1);

	/*CBaseAnimating *pAnimating = pPlayer->GetBaseAnimating();
	if (pAnimating)
	{
		int bg_rh = pAnimating->FindBodygroupByName("right_hand");
		int bg_lh = pAnimating->FindBodygroupByName("left_hand");
		pAnimating->SetBodygroup(bg_rh, 1);
		pAnimating->SetBodygroup(bg_lh, 1);
	}*/
}

void CPlayerModel::SetOrigin(const Vector &origin, const QAngle &origAngle)
{
	SetLocalOrigin(origin);
	SetLocalAngles(origAngle);
}
//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void CPlayerModel::Precache(void)
{
	PrecacheModel(cvar->FindVar("oc_playermodel_suit")->GetString());
}

void CPlayerModel::Think(void)
{
	//CHL2_Player *pPlayer = (CHL2_Player *)GetOwnerEntity();

	CBasePlayer *pPlayer = ToBasePlayer(GetOwnerEntity());
	if (!pPlayer)
		return;

	CBaseViewModel *vm = pPlayer->GetViewModel(0);
	if (vm)
	{

		if (GetParent() != vm)
		{
			SetParent(vm);
			newOrigin = vm->GetLocalOrigin();
			newOrigin.x = oc_player_upperhalf_origin_x.GetFloat();
			newOrigin.y = oc_player_upperhalf_origin_y.GetFloat();
			newOrigin.z = oc_player_upperhalf_origin_z.GetFloat();
			SetLocalOrigin(newOrigin);

			//vm->SetParent(pPlayer->GetBaseAnimating(), LookupAttachment("eyes"));
		}
		if (pPlayer->GetActiveWeapon() && pPlayer->GetActiveWeapon()->GetParent() != this)
		{
			//pPlayer->GetActiveWeapon()->FollowEntity(this);

			pPlayer->GetActiveWeapon()->SetAbsVelocity(vec3_origin);
			pPlayer->GetActiveWeapon()->RemoveSolidFlags(FSOLID_TRIGGER);
			pPlayer->GetActiveWeapon()->FollowEntity(this);
			pPlayer->GetActiveWeapon()->SetOwner((CBaseCombatCharacter*)this->GetOwnerEntity());
			pPlayer->GetActiveWeapon()->SetOwnerEntity(this);
			pPlayer->GetActiveWeapon()->RemoveEffects(EF_ITEM_BLINK);
		}

		Vector attachment;
		pPlayer->GetAttachment("eyes", attachment);
		vm->SetAbsOrigin(attachment);

		//if (oc_player_apply_offsets.GetInt())
		//{
			newOrigin = vm->GetLocalOrigin();
			newOrigin.x = oc_player_upperhalf_origin_x.GetFloat();
			newOrigin.y = oc_player_upperhalf_origin_y.GetFloat();
			newOrigin.z = oc_player_upperhalf_origin_z.GetFloat();

			SetLocalOrigin(newOrigin);
			SetViewOffset(vm->GetLocalOrigin());
		//}
		//oc_player_apply_offsets.SetValue(0);


		//QAngle eyes = pPlayer->EyeAngles();

		//float crouchDelay = 1.0f;
		//const float border = 20.f;
		if ((pPlayer->m_afButtonPressed & IN_DUCK))
		{
			GetBaseAnimating()->m_flPlaybackRate = 1.0;
			GetBaseAnimating()->ResetSequence(ACT_VM_RELOAD);
			GetBaseAnimating()->SetCycle(0);
		}

		if ((pPlayer->m_nButtons & IN_DUCK) && !inCrouch)
		{
			//newOrigin.z = oc_player_upperhalf_origin_z.GetFloat() + 15.f;
			//SetLocalOrigin(newOrigin);
			posZ = 0;
			//pPlayer->m_flPlayerTimeNextAnim = gpGlobals->curtime + crouchDelay;
			inCrouch = true;
		}
		else if (!(pPlayer->m_nButtons & IN_DUCK) && inCrouch)
		{
			//newOrigin.z = oc_player_upperhalf_origin_z.GetFloat();
			//SetLocalOrigin(newOrigin);
			posZ = 0;
			//pPlayer->m_flPlayerTimeNextAnim = gpGlobals->curtime + crouchDelay;
			inCrouch = false;
		}

		//if (pPlayer->m_flPlayerTimeNextAnim >= gpGlobals->curtime)
		{
			//float _time = crouchDelay - (pPlayer->m_flPlayerTimeNextAnim - gpGlobals->curtime);
			//int direction = (pPlayer->m_nButtons & IN_DUCK) ? 1 : -1;
			//float value = 15.f * (float)direction;

			//if (direction > 0)
				//posZ = Lerp(_time, 0.f, value);
			//else
				//posZ = Lerp(_time, value, 0.f);

			//newOrigin.z = oc_player_upperhalf_origin_z.GetFloat() + posZ;
			//SetLocalOrigin(newOrigin);
		}


	}
}