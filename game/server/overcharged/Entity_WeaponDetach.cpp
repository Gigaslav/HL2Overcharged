#include "cbase.h"
#include "engine/IEngineSound.h"
#include "in_buttons.h"
#include "ammodef.h"
#include "IEffects.h"
#include "beam_shared.h"
#include "weapon_gauss.h"
#include "soundenvelope.h"
#include "decals.h"
#include "soundent.h"
#include "grenade_ar2.h"
#include "te_effect_dispatch.h"
#include "hl2_player.h"
#include "ndebugoverlay.h"
#include "movevars_shared.h"
#include "bone_setup.h"
#include "ai_basenpc.h"
#include "ai_hint.h"
#include "npc_crow.h"
#include "globalstate.h"
#include "vehicle_jeep.h"
#include "eventqueue.h"
#include "rumble_shared.h"
#include "baseanimating.h"
#include "entity_weapondetach.h"
#include "haptics/haptic_utils.h"
// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

LINK_ENTITY_TO_CLASS(ent_weapon_detach, CEntityWeaponDetach);

BEGIN_DATADESC(CEntityWeaponDetach)
DEFINE_FIELD(foundedPlayer, FIELD_BOOLEAN),
DEFINE_FIELD(count, FIELD_INTEGER),
DEFINE_FIELD(canTakeOffTheGun, FIELD_BOOLEAN),
END_DATADESC()

CEntityWeaponDetach::CEntityWeaponDetach()
{

}
CEntityWeaponDetach::CEntityWeaponDetach(CBaseEntity *pParent, const char *attachment, const Vector &min, const Vector &max)
{
	/*if (gEntList.FindEntityByClassname(this, "ent_weapon_detach") != NULL)
		delete gEntList.FindEntityByClassname(this, "ent_weapon_detach");*/

	SetParent(pParent);
	int attachment1 = pParent->GetBaseAnimating()->LookupAttachment(attachment);
	Vector vecOrigin2;
	pParent->GetBaseAnimating()->GetAttachment(attachment1, vecOrigin2);
	Vector pEnt, vForward, vRight, vUp;
	AngleVectors(GetAbsAngles(), &vForward, &vRight, &vUp);
	pEnt = GetAbsOrigin() + (vForward * 64) + (vUp * 64);

	SetAbsOrigin(pEnt);
	SetCollisionGroup(COLLISION_GROUP_PLAYER);
	SetSolid(SOLID_BBOX);
	AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	SetMoveType(MOVETYPE_NONE);
	SetOwnerEntity(pParent);

	//entCreate(pParent, pEnt, min, max);
	UTIL_SetSize(this, min, max);

}

CEntityWeaponDetach *CEntityWeaponDetach::entCreate(CBaseEntity *pOwner, const Vector &position, const Vector &min, const Vector &max)//, const Vector &min, const Vector &max)
{
	CEntityWeaponDetach *pEnt = CREATE_ENTITY(CEntityWeaponDetach, "ent_weapon_detach");
	pEnt->SetAbsOrigin(position);
	pEnt->SetCollisionGroup(COLLISION_GROUP_PLAYER);
	pEnt->SetSolid(SOLID_BBOX);
	pEnt->AddSolidFlags(FSOLID_TRIGGER | FSOLID_NOT_SOLID);
	pEnt->SetMoveType(MOVETYPE_NONE);
	pEnt->SetOwnerEntity(pOwner);

	UTIL_SetSize(pEnt, min, max);


	return pEnt;
}

void CEntityWeaponDetach::LegacyThink(CBaseEntity *pVehicle, const char *pWeapon, const char *attachment, const char *textOn, const char *textOff, float radius, bool alreadyHaveIt)
{
	if (pVehicle == NULL || pVehicle->GetBaseAnimating() == NULL)
	{
		delete this;
		return;
	}


	/*if (gEntList.FindEntityByClassname(this, GetClassname()))
	{
		DevMsg("Founded old \n");
		UTIL_RemoveImmediate(gEntList.FindEntityByClassname(this, GetClassname()));
		if (gEntList.FindEntityByClassname(this, GetClassname()) != NULL)
			delete gEntList.FindEntityByClassname(this, GetClassname());
	}*/

	CBaseServerVehicle *pServerVehicle = dynamic_cast<CBaseServerVehicle *>(pVehicle->GetServerVehicle());
	if (pServerVehicle)
	{
		int iattachment = pVehicle->GetBaseAnimating()->LookupAttachment(attachment);
		Vector vecOrigin;
		QAngle ang;
		pVehicle->GetBaseAnimating()->GetAttachment(iattachment, vecOrigin, ang);

		this->SetAbsOrigin(vecOrigin);
		this->SetAbsAngles(ang);
		//if (UTIL_GetLocalPlayer() != NULL && this->GetAbsOrigin().DistTo(UTIL_GetLocalPlayer()->GetAbsOrigin()) >= radius)
		//canTakeOffTheGun = false;

		if (!pServerVehicle->GetPassenger() && UTIL_GetLocalPlayer() != NULL && UTIL_GetLocalPlayer()->FInViewCone(this))
		{

			CBaseEntity *ppEnts[512];
			int nEntCount = UTIL_EntitiesInSphere(ppEnts, 512, vecOrigin, radius, 0);

			for (int i = 0; i < nEntCount && !foundedPlayer; i++)
			{
				canTakeOffTheGun = false;
				if (ppEnts[i] == NULL)
				{
					continue;
				}

				if (ppEnts[i] != NULL)
				{

					if (ppEnts[i]->IsPlayer())
					{
						CBasePlayer *pPlayer = ToBasePlayer(ppEnts[i]);
						if (pPlayer != NULL)
						{
							if (this != NULL)
							{
								if (pPlayer->FInViewCone(this))
								{
									if (alreadyHaveIt)
										UTIL_HudHintText(pPlayer, textOn);//engine->Con_NPrintf(3, textOn);
									else
									{
										if (!pPlayer->HasNamedPlayerItem(pWeapon))
											UTIL_HudHintText(pPlayer, textOff);//engine->Con_NPrintf(3, textOff);
									}
									
									if (!canTakeOffTheGun)
									{
										canTakeOffTheGun = true;
										ppEnts[i] = NULL;
									}

								}
								else
								{
									if (canTakeOffTheGun)
										canTakeOffTheGun = false;
								}
							}
						}

						foundedPlayer = true;
						break;
					}
				}

			}

			if (foundedPlayer)
				count++;
			if (count > 17)
			{
				foundedPlayer = false;
				count = 0;
			}
		}
		else if (!pServerVehicle->GetPassenger() && UTIL_GetLocalPlayer() != NULL && !UTIL_GetLocalPlayer()->FInViewCone(this))
			canTakeOffTheGun = false;
	}
	else
	{
		delete this;
	}

	//DevMsg("alreadyHaveIt %i \n", alreadyHaveIt);
	//DevMsg("canTakeOffTheGun %i \n", canTakeOffTheGun);
	//DevMsg("FInViewCone box %i \n", UTIL_GetLocalPlayer()->FInViewCone(this));
}